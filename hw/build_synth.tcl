# Vivado synthesis/implementation baseline for Virtex UltraScale+.
# Run with: vivado -mode batch -source hw/build_synth.tcl
set_part xcvu9p-flga2104-2L-e
create_project photon_synth ./vivado_build -part xcvu9p-flga2104-2L-e -force
read_verilog -sv hw/fpga_pre_trade_risk_gate.sv
read_verilog -sv hw/fpga_tcp_offload_engine.sv
synth_design -top fpga_pre_trade_risk_gate -part xcvu9p-flga2104-2L-e -mode out_of_context
create_clock -period 3.103 [get_ports clk]
opt_design
place_design
phys_opt_design
route_design
report_timing_summary -file vivado_build/risk_gate_timing.rpt
report_utilization -file vivado_build/risk_gate_utilization.rpt
if {[get_property SLACK [get_timing_paths -max_paths 1]] < 0} {
  error "Timing target failed: negative worst slack"
}
close_project
