#!/usr/bin/env bash
set -euo pipefail
# Review hardware topology before applying these settings.
sudo systemctl disable --now irqbalance || true
sudo cpupower frequency-set -g performance || true
echo 0 | sudo tee /proc/sys/kernel/sched_rt_runtime_us >/dev/null
echo "Recommended GRUB: isolcpus=2-15 nohz_full=2-15 rcu_nocbs=2-15 processor.max_cstate=0 idle=poll mce=off transparent_hugepage=never iommu=pt"
echo "Pin NIC IRQs to CPUs 0-1 after inspecting /proc/interrupts."
