# Copyright 1991-2007 Mentor Graphics Corporation
# 
# Modification by Oklahoma State University
# Use with Testbench 
# James Stine, 2008
# Go Cowboys!!!!!!
#
# All Rights Reserved.
#
# THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION
# WHICH IS THE PROPERTY OF MENTOR GRAPHICS CORPORATION
# OR ITS LICENSORS AND IS SUBJECT TO LICENSE TERMS.

# Use this run.do file to run this example.
# Either bring up ModelSim and type the following at the "ModelSim>" prompt:
#     do run.do
# or, to run from a shell, type the following at the shell prompt:
#     vsim -do run.do -c
# (omit the "-c" to see the GUI while running from the shell)

onbreak {resume}

# create library
if [file exists work] {
    vdel -all
}
vlib work

# compile source files
vlog sdes.sv test_sdes.sv

# start and run simulation
vsim -voptargs=+acc work.tb

view wave

-- display input and output signals as hexidecimal values
# Diplays All Signals recursively
# add wave -hex -r /tb/*
add wave -hex /tb/dut1/plaintext
add wave -hex /tb/dut1/key
add wave -hex /tb/dut1/IP_out
add wave -hex /tb/dut1/K1
add wave -hex /tb/dut1/K2
add wave -hex /tb/dut1/K1_out
add wave -hex /tb/dut1/K2_out
add wave -hex /tb/dut1/sw_out
add wave -hex /tb/dut1/ciphertext
add wave -noupdate -divider -height 32 "f_K (Feistel) Output"
add wave -hex /tb/dut1/feistel_K1_out
add wave -hex /tb/dut1/feistel_K2_out
add wave -noupdate -divider -height 32 "sbox0/sbox1 encrypt for each K1/K2"
add wave -hex /tb/dut1/f1/inp_block
add wave -hex /tb/dut1/f1/key
add wave -hex /tb/dut1/f1/first_chunk
add wave -hex /tb/dut1/f1/second_chunk
add wave -hex /tb/dut1/f1/xor_fout
add wave -hex /tb/dut1/f1/xor_f1
add wave -hex /tb/dut1/f1/xor_f2
add wave -hex /tb/dut1/f1/p4_in
add wave -hex /tb/dut1/f1/p4_out
add wave -hex /tb/dut1/f1/EP_out
add wave -hex /tb/dut1/f1/xor_out
add wave -hex /tb/dut1/f1/s0_out
add wave -hex /tb/dut1/f1/s1_out
add wave -hex /tb/dut1/f1/out_block
add wave -noupdate -divider -height 32 "sbox0/sbox1 decrypt"
add wave -hex /tb/dut1/f2/inp_block
add wave -hex /tb/dut1/f2/key
add wave -hex /tb/dut1/f2/first_chunk
add wave -hex /tb/dut1/f2/second_chunk
add wave -hex /tb/dut1/f2/xor_fout
add wave -hex /tb/dut1/f2/xor_f2
add wave -hex /tb/dut1/f2/xor_f2
add wave -hex /tb/dut1/f2/p4_in
add wave -hex /tb/dut1/f2/p4_out
add wave -hex /tb/dut1/f2/EP_out
add wave -hex /tb/dut1/f2/xor_out
add wave -hex /tb/dut1/f2/s0_out
add wave -hex /tb/dut1/f2/s1_out
add wave -hex /tb/dut1/f2/out_block





-- Set Wave Output Items 
TreeUpdate [SetDefaultTree]
WaveRestoreZoom {0 ps} {75 ns}
configure wave -namecolwidth 350
configure wave -valuecolwidth 250
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2

-- Run the Simulation
run 99999338ns


