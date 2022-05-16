# DES
Open-source DES library in SystemVerilog

james.stine@okstate.edu 16 May 2022

This has two projects completed in ECEN 3233 Digital Logic Design at Oklahoma State University.  The S-DES is complete as it was utilized during the Fall 2021 semester.  The DES idea is used during the Spring 2022 semester.

The laboraty deals with implementing either S-DES or DES and then the Project involves cracking the key given the plaintext and ciphertext. The S-DES contains the cracker and multiple keys may work given a known ciphertext and plaintext.  The FSM is designed to continue checking for the key even after a hit although the original FSM discussed within the class stops after it finds key.  The main S-DES and DES unit is designed as a parallel unit (i.e., using combinational logic) but could be designed with pipelining in mind.  

All of the designs are implemented on the National Instruments Zynq 7000 enabled Digital System Development Board (DSDB) but can easily be ported to any
implementation (System on Chip or FPGA).



