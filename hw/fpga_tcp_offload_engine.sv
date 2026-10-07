module fpga_tcp_offload_engine #(
  parameter int DATA_W = 64
)(
  input  logic                  clk,
  input  logic                  rst,

  input  logic                  start_i,
  input  logic [47:0]           dst_mac_i,
  input  logic [47:0]           src_mac_i,
  input  logic [31:0]           src_ip_i,
  input  logic [31:0]           dst_ip_i,
  input  logic [15:0]           src_port_i,
  input  logic [15:0]           dst_port_i,
  input  logic [31:0]           seq_i,
  input  logic [31:0]           ack_i,
  input  logic [7:0]            tcp_flags_i,
  input  logic [15:0]           window_i,
  input  logic [15:0]           payload_len_i,
  // One's-complement 16-bit sum of the payload, supplied by the ingress
  // checksum pipeline while payload remains in registered DMA memory.
  input  logic [31:0]           payload_checksum_i,

  input  logic                  payload_valid_i,
  input  logic [DATA_W-1:0]     payload_data_i,
  input  logic [DATA_W/8-1:0]   payload_keep_i,
  input  logic                  payload_last_i,
  output logic                  payload_ready_o,

  output logic                  out_valid,
  output logic [DATA_W-1:0]     out_data,
  output logic [DATA_W/8-1:0]   out_keep,
  output logic                  out_last,
  input  logic                  out_ready
);

  localparam int BYTES = DATA_W / 8;
  localparam logic [15:0] ETHERTYPE_IPV4 = 16'h0800;

  typedef enum logic [2:0] {IDLE, PRIME, HEADER, PAYLOAD, DONE} state_t;
  state_t state;

  logic [7:0] header [0:53];
  logic [5:0] header_pos;
  logic [DATA_W-1:0] pending_data;
  logic [BYTES-1:0] pending_keep;
  logic pending_valid;
  logic pending_last;

  function automatic [15:0] fold16(input logic [31:0] x);
    logic [31:0] y;
    begin
      y = x;
      y = (y[15:0] + y[31:16]);
      y = (y[15:0] + y[31:16]);
      fold16 = y[15:0];
    end
  endfunction

  function automatic [15:0] csum_add(input logic [15:0] a, input logic [15:0] b);
    logic [16:0] s;
    begin
      s = {1'b0,a} + {1'b0,b};
      csum_add = s[15:0] + s[16];
    end
  endfunction

  logic [15:0] ip_checksum;
  logic [15:0] tcp_checksum;
  logic [15:0] ip_total_len;
  logic [15:0] tcp_total_len;
  logic [31:0] ip_sum;
  logic [31:0] tcp_sum;

  always_comb begin
    ip_total_len = 16'(20 + 20 + payload_len_i);
    tcp_total_len = 16'(20 + payload_len_i);

    ip_sum = 32'h0000;
    ip_sum += 32'h4500;
    ip_sum += {16'h0000, ip_total_len};
    ip_sum += 32'h0000; // identification
    ip_sum += 32'h4000; // DF, zero fragment offset
    ip_sum += 32'h4006; // TTL 64, TCP
    ip_sum += {16'h0000, src_ip_i[31:16]};
    ip_sum += {16'h0000, src_ip_i[15:0]};
    ip_sum += {16'h0000, dst_ip_i[31:16]};
    ip_sum += {16'h0000, dst_ip_i[15:0]};
    ip_checksum = ~fold16(ip_sum);

    tcp_sum = 32'h0000;
    tcp_sum += {16'h0000, src_ip_i[31:16]};
    tcp_sum += {16'h0000, src_ip_i[15:0]};
    tcp_sum += {16'h0000, dst_ip_i[31:16]};
    tcp_sum += {16'h0000, dst_ip_i[15:0]};
    tcp_sum += 32'h0006_0000;
    tcp_sum += {16'h0000, tcp_total_len};
    tcp_sum += {16'h0000, src_port_i};
    tcp_sum += {16'h0000, dst_port_i};
    tcp_sum += {16'h0000, seq_i[31:16]};
    tcp_sum += {16'h0000, seq_i[15:0]};
    tcp_sum += {16'h0000, ack_i[31:16]};
    tcp_sum += {16'h0000, ack_i[15:0]};
    tcp_sum += 32'h0000_5000 | {24'h0, tcp_flags_i};
    tcp_sum += {16'h0000, window_i};
    tcp_sum += payload_checksum_i;
    tcp_checksum = ~fold16(tcp_sum);

    header[0]  = dst_mac_i[47:40];
    header[1]  = dst_mac_i[39:32];
    header[2]  = dst_mac_i[31:24];
    header[3]  = dst_mac_i[23:16];
    header[4]  = dst_mac_i[15:8];
    header[5]  = dst_mac_i[7:0];
    header[6]  = src_mac_i[47:40];
    header[7]  = src_mac_i[39:32];
    header[8]  = src_mac_i[31:24];
    header[9]  = src_mac_i[23:16];
    header[10] = src_mac_i[15:8];
    header[11] = src_mac_i[7:0];
    header[12] = ETHERTYPE_IPV4[15:8];
    header[13] = ETHERTYPE_IPV4[7:0];

    header[14] = 8'h45;
    header[15] = 8'h00;
    header[16] = ip_total_len[15:8];
    header[17] = ip_total_len[7:0];
    header[18] = 8'h00; header[19] = 8'h00;
    header[20] = 8'h40; header[21] = 8'h00;
    header[22] = 8'h40; header[23] = 8'h06;
    header[24] = ip_checksum[15:8];
    header[25] = ip_checksum[7:0];
    header[26] = src_ip_i[31:24];
    header[27] = src_ip_i[23:16];
    header[28] = src_ip_i[15:8];
    header[29] = src_ip_i[7:0];
    header[30] = dst_ip_i[31:24];
    header[31] = dst_ip_i[23:16];
    header[32] = dst_ip_i[15:8];
    header[33] = dst_ip_i[7:0];

    header[34] = src_port_i[15:8];
    header[35] = src_port_i[7:0];
    header[36] = dst_port_i[15:8];
    header[37] = dst_port_i[7:0];
    header[38] = seq_i[31:24];
    header[39] = seq_i[23:16];
    header[40] = seq_i[15:8];
    header[41] = seq_i[7:0];
    header[42] = ack_i[31:24];
    header[43] = ack_i[23:16];
    header[44] = ack_i[15:8];
    header[45] = ack_i[7:0];
    header[46] = 8'h50;
    header[47] = tcp_flags_i;
    header[48] = window_i[15:8];
    header[49] = window_i[7:0];
    header[50] = tcp_checksum[15:8];
    header[51] = tcp_checksum[7:0];
    header[52] = 8'h00;
    header[53] = 8'h00;
  end

  always_comb begin
    payload_ready_o = (state == PRIME) ||
                      (state == PAYLOAD && (!out_valid || out_ready));
  end

  integer k;
  always_ff @(posedge clk) begin
    if (rst) begin
      state <= IDLE;
      header_pos <= 0;
      out_valid <= 1'b0;
      out_data <= '0;
      out_keep <= '0;
      out_last <= 1'b0;
      pending_data <= '0;
      pending_keep <= '0;
      pending_valid <= 1'b0;
      pending_last <= 1'b0;
    end else begin
      case (state)
        IDLE: begin
          out_valid <= 1'b0;
          if (start_i) begin
            header_pos <= 0;
            pending_valid <= 1'b0;
            state <= (payload_len_i == 0) ? HEADER : PRIME;
          end
        end

        PRIME: begin
          if (payload_valid_i && payload_ready_o) begin
            pending_data <= payload_data_i;
            pending_keep <= payload_keep_i;
            pending_last <= payload_last_i;
            pending_valid <= 1'b1;
            header_pos <= 0;
            state <= HEADER;
          end
        end

        HEADER: begin
          if (!out_valid || out_ready) begin
            out_valid <= 1'b1;
            out_data <= '0;
            out_keep <= '0;
            out_last <= 1'b0;
            for (k = 0; k < BYTES; k = k + 1) begin
              if ((header_pos + k) < 54) begin
                out_data[k*8 +: 8] <= header[header_pos + k];
                out_keep[k] <= 1'b1;
              end else if (pending_valid && ((header_pos + k) == 54)) begin
                out_data[k*8 +: 8] <= pending_data[7:0];
                out_keep[k] <= pending_keep[0];
              end else if (pending_valid && ((header_pos + k) == 55)) begin
                out_data[k*8 +: 8] <= pending_data[15:8];
                out_keep[k] <= pending_keep[1];
              end
            end

            if (header_pos + BYTES >= 54) begin
              if (payload_len_i == 0) begin
                out_last <= 1'b1;
                state <= DONE;
              end else if (pending_valid) begin
                if (pending_last && pending_keep <= 2) begin
                  out_last <= 1'b1;
                  state <= DONE;
                end else begin
                  pending_data <= pending_data >> 16;
                  pending_keep <= pending_keep >> 2;
                  state <= PAYLOAD;
                end
              end
            end else begin
              header_pos <= header_pos + BYTES;
            end
          end
        end

        PAYLOAD: begin
          if (!out_valid || out_ready) begin
            if (pending_valid) begin
              out_valid <= 1'b1;
              out_data <= pending_data;
              out_keep <= pending_keep;
              out_last <= pending_last;
              pending_valid <= 1'b0;
              if (pending_last) state <= DONE;
            end else if (payload_valid_i) begin
              out_valid <= 1'b1;
              out_data <= payload_data_i;
              out_keep <= payload_keep_i;
              out_last <= payload_last_i;
              if (payload_last_i) state <= DONE;
            end
          end
        end

        DONE: begin
          out_valid <= 1'b0;
          out_last <= 1'b0;
          state <= IDLE;
        end
        default: state <= IDLE;
      endcase
    end
  end
endmodule
