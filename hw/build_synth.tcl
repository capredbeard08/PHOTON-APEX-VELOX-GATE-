# Vivado batch implementation baseline for Virtex UltraScale+.
set part xcvu9p-flga2104-2L-e
create_project photon_synth ./vivado_build -part $part -force
foreach src {hw/fpga_pre_trade_risk_gate.sv hw/fpga_tcp_offload_engine.sv} {
  read_verilog -sv $src
}
synth_design -top fpga_pre_trade_risk_gate -part $part -mode out_of_context
write_checkpoint -force vivado_build/risk_gate_synth.dcp
report_utilization -file vivado_build/risk_gate_utilization.rpt
close_design
synth_design -top fpga_tcp_offload_engine -part $part -mode out_of_context
create_clock -period 3.103 [get_ports clk]
opt_design
place_design
phys_opt_design
route_design
report_timing_summary -delay_type max -file vivado_build/tcp_timing.rpt
report_utilization -file vivado_build/tcp_utilization.rpt
set paths [get_timing_paths -max_paths 1 -quiet]
if {[llength $paths] > 0} {
  set wns [get_property SLACK [lindex $paths 0]]
  if {$wns < 0} { error "Timing target failed: WNS=$wns ns" }
}
close_design
close_project
