onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /stimulus/dut/b2/inp_block
add wave -noupdate /stimulus/dut/b2/subkey
add wave -noupdate /stimulus/dut/b2/fk/inp_block
add wave -noupdate /stimulus/dut/b2/fk/ep_out
add wave -noupdate /stimulus/dut/b2/fk/sbox_pre
add wave -noupdate /stimulus/dut/b2/fk/sb1out
add wave -noupdate /stimulus/dut/b2/fk/sb2out
add wave -noupdate /stimulus/dut/b2/fk/sb3out
add wave -noupdate /stimulus/dut/b2/fk/sb4out
add wave -noupdate /stimulus/dut/b2/fk/sb5out
add wave -noupdate /stimulus/dut/b2/fk/sb6out
add wave -noupdate /stimulus/dut/b2/fk/sb7out
add wave -noupdate /stimulus/dut/b2/fk/sb8out
add wave -noupdate -expand /stimulus/dut/b2/fk/b2/inp_block
add wave -noupdate -expand /stimulus/dut/b2/fk/out_block
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {1973 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 150
configure wave -valuecolwidth 289
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {0 ps} {74188 ps}
