module tb_fpga_pre_trade_risk_gate;
  logic clk=0, rst=1, valid_i;
  logic [31:0] qty_i,max_qty_i;
  logic [63:0] price_i,min_price_i,max_price_i;
  logic [127:0] max_notional_i;
  logic valid_o,accept_o,abort_frame_o;
  always #1.551 clk = ~clk;
  fpga_pre_trade_risk_gate dut(.*);
  task automatic check(input [31:0] q,input [63:0] p,input [31:0] mq,
                       input [63:0] lo,input [63:0] hi,input [127:0] mn,
                       input bit expected);
    begin
      valid_i=1; qty_i=q; price_i=p; max_qty_i=mq; min_price_i=lo; max_price_i=hi; max_notional_i=mn;
      #0.2;
      if (accept_o !== expected) $fatal(1,"risk mismatch q=%0d p=%0d",q,p);
    end
  endtask
  initial begin
    valid_i=0; qty_i=0; price_i=0; max_qty_i=100; min_price_i=10; max_price_i=1000; max_notional_i=100000;
    #5 rst=0;
    check(50,100,100,10,1000,100000,1);
    check(101,100,100,10,1000,100000,0);
    check(50,9,100,10,1000,100000,0);
    check(50,1001,100,10,1000,100000,0);
    check(2000,100,5000,10,1000,100000,0);
    $display("RISK_GATE_TESTS_PASS");
    $finish;
  end
endmodule
