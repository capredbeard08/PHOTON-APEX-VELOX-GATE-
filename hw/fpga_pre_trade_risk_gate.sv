module fpga_pre_trade_risk_gate (
  input  logic         clk,
  input  logic         rst,
  input  logic         valid_i,
  input  logic [31:0]  qty_i,
  input  logic [63:0]  price_i,
  input  logic [31:0]  max_qty_i,
  input  logic [63:0]  min_price_i,
  input  logic [63:0]  max_price_i,
  input  logic [127:0] max_notional_i,
  output logic         valid_o,
  output logic         accept_o,
  output logic         abort_frame_o
);
  logic [127:0] qty_ext;
  logic [127:0] price_ext;
  logic [127:0] notional;
  logic bad_qty, bad_min, bad_max, bad_notional;

  always_comb begin
    qty_ext = {96'd0, qty_i};
    price_ext = {64'd0, price_i};
    notional = qty_ext * price_ext;
    bad_qty = qty_i > max_qty_i;
    bad_min = price_i < min_price_i;
    bad_max = price_i > max_price_i;
    bad_notional = notional > max_notional_i;

    valid_o = valid_i;
    abort_frame_o = valid_i && (bad_qty || bad_min || bad_max || bad_notional);
    accept_o = valid_i && !abort_frame_o;
  end
endmodule
