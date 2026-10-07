module tb_fpga_tcp_offload_engine;
  logic clk=0, rst=1, start_i;
  logic [47:0] dst_mac_i, src_mac_i;
  logic [31:0] src_ip_i, dst_ip_i, seq_i, ack_i;
  logic [15:0] src_port_i, dst_port_i, window_i, payload_len_i;
  logic [7:0] tcp_flags_i;
  logic [31:0] payload_checksum_i;
  logic payload_valid_i, payload_last_i, payload_ready_o;
  logic [63:0] payload_data_i;
  logic [7:0] payload_keep_i;
  logic out_valid, out_last, out_ready;
  logic [63:0] out_data;
  logic [7:0] out_keep;

  always #1.551 clk = ~clk;

  fpga_tcp_offload_engine dut(.*);

  initial begin
    start_i=0; dst_mac_i=48'h001122334455; src_mac_i=48'h66778899aabb;
    src_ip_i=32'hc0000201; dst_ip_i=32'hc0000202;
    src_port_i=16'd5000; dst_port_i=16'd6000;
    seq_i=32'd1; ack_i=32'd2; tcp_flags_i=8'h18;
    window_i=16'h4000; payload_len_i=0; payload_checksum_i=0;
    payload_valid_i=0; payload_data_i=0; payload_keep_i=0; payload_last_i=0;
    out_ready=1;
    #5 rst=0;
    #2 start_i=1;
    #2 start_i=0;
    wait(out_valid);
    if (out_keep !== 8'hff) $fatal(1,"unexpected header keep");
    @(posedge clk);
    if (!out_valid || !out_last) $fatal(1,"zero-payload frame did not terminate");
    $display("TCP_OFFLOAD_TEST_PASS");
    $finish;
  end
endmodule
