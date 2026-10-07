module fpga_tcp_offload_engine #(
  parameter int DATA_W = 64
)(
  input  logic                 clk,
  input  logic                 rst,
  input  logic                 in_valid,
  input  logic [DATA_W-1:0]    in_data,
  input  logic [DATA_W/8-1:0]  in_keep,
  input  logic                 in_last,
  output logic                 in_ready,
  output logic                 out_valid,
  output logic [DATA_W-1:0]    out_data,
  output logic [DATA_W/8-1:0]  out_keep,
  output logic                 out_last,
  input  logic                 out_ready
);
  assign in_ready = out_ready || !out_valid;
  always_ff @(posedge clk) begin
    if (rst) begin
      out_valid <= 1'b0; out_data <= '0; out_keep <= '0; out_last <= 1'b0;
    end else if (in_ready) begin
      out_valid <= in_valid;
      out_data <= in_data;
      out_keep <= in_keep;
      out_last <= in_last;
    end
  end
endmodule
