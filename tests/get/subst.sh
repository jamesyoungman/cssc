#! /bin/sh

# Some substitution tests...


# Import common functions & definitions.
. ../common/test-common


# Get a test file...
s=s.keywords.txt
remove $s
uudecode < keywords.uue || miscarry could not extract test file.

expands_to () {
    # $1 -- label
    # $2 -- format
    # $3 -- expansion
    # $4 -- stderr
docommand $1 "${get} -p $s | grep \"^_${2}_ \" " 0 "$3" "$stderr"
}

stderr="1.1\n83 lines\n"

expands_to A A "_A_ @(#) keywords.txt 1.1@(#)\n" 
expands_to B B "_B_ 0\n"                         
expands_to C C "_C_ 16\n_C_ 17\n"                
expands_to E E "_E_ 97/10/25\n"                  
expands_to F F "_F_ s.keywords.txt\n"            
expands_to G G "_G_ 10/25/97\n"                  
expands_to I I "_I_ 1.1.0.0\n"                   
expands_to L L "_L_ 1\n"                         
expands_to M M "_M_ keywords.txt\n"	         
expands_to P P "_P_ ${PWD}/s.keywords.txt\n"     
expands_to Q Q "_Q_ \n"                          
expands_to R R "_R_ 1\n"                         
expands_to S S "_S_ 0\n"                         
expands_to U U "_U_ 13:06:02\n"                  
expands_to W W "_W_ @(#)keywords.txt	1.1"     
expands_to Y Y "_Y_ \n"                          
expands_to Z Z "_Z_ @(#)\n"                      

# TODO: better tests for Q, D, H, T, Y


# tests are finished.
remove $s
success
