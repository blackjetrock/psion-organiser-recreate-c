#!/usr/bin/tclsh

set f [glob *.opl]

puts $f

foreach fn [split $f " "] {
    puts $fn
}
