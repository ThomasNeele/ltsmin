# include.tcl
# File for putting general test procedures

# Models should be able to complete whithin the timeout value.
set timeout 120

# The directory containing all the models used for testing.
set EXAMPLES_PATH "$base_dir/../examples"

# filter: filter a specific (list of) backend(s): {mc,sym,seq}
proc find_alg_backends { filter } {
    global base_dir
    set backends [list]
    set lts_backends [glob -directory "$base_dir/../src" -type d pins2lts-$filter ]
    foreach dir $lts_backends {
        set lts_bins [glob -nocomplain -directory $dir -type f *2lts-$filter ]
        foreach path $lts_bins {
            lappend backends $path
        }
    }

    return $backends
}

proc find_lang_frontends { filter } {
    global base_dir
    set frontends [list]
    set lts_backends [glob -directory "$base_dir/../src" -type d pins2lts* ]
    foreach dir $lts_backends {
        set lts_bins [glob -nocomplain -directory $dir -type f $filter]
        foreach path $lts_bins {
            lappend frontends $path
        }
    }

    return $frontends
}

proc runmytest { test_name command_line exp_output} {
    send_user "starting $command_line\n"

    # NOTE: this is ugly. If the exp_output is not set, put an unfindable string in it.
    set unfindable_string "adhadkhaslkdLKHLKHads^*&^876"

    if { [string length $exp_output] == 0 } {
        set exp_output unfindable_string
    }

    eval spawn $command_line

    expect {

        # expected last line when execution succeeds
        "writing output took" {
            pass "Program finished\n"
        }

        "not enough arguments" {
            fail "Not enough arguments given for $command_line"
        }

        "File does not exist:" {
            fail "Argument file does not exists"
        }


        "Zobrist and treedbs is not implemented" {
            xfail "The combination of zobrist and treedbs is not implemented";
            return
        }

        # Check for any warning messages in the output first
        Warning {
            fail "$test_name: warning: $expect_out(buffer)"
        }

        # Check for any error messages
        ERROR {
            fail "$test_name: error: $expect_out(buffer)"
        }

         "error" {
             fail "An error message was encountered in the application output.";
         }

        timeout {
            fail "Program takes to long to execute"
            return
        }

        $exp_output {
            pass "Expected output $exp_output found"
        }
        
        eof {
            # only fail if an exp_output is set, otherwise let the exit code decide
            if { $exp_output != $unfindable_string } {
                fail "Program ended without outputting values which are known to be correct"
            }        
        }

    }
    # get the exit code
    set result [wait]

    # check for non-zero exit codes
    set exit_code [lindex $result 3]
    #puts "DEBUG: exit_code: $exit_code"
    switch $exit_code {
        0   {
                if { $exp_output != $unfindable_string } {
                    return
            }
                pass  "$test_name: Program exited with a zero exit code" 
            }
        1   { xfail "$test_name: Program exited with LTSMIN_COUNTER_EXAMPLE" }
        127 { fail "$test_name: Program exited with 127. Probably unable to find external lib. Run ldconfig or update your \$LD_LIBRARY_PATH" }
        255 { fail  "$test_name: Program exited with LTSMIN_EXIT_ERROR" }
        default { fail "$test_name: Program exited with a unknown exit code: $exit_code" }
    }
}

# create a list with for every bin the path
set binpaths(ltsmin-compare) "$base_dir/../src/ltsmin-compare/ltsmin-compare"
set binpaths(ltsmin-convert) "$base_dir/../src/ltsmin-convert/ltsmin-convert"
set binpaths(ltsmin-printtrace) "$base_dir/../src/ltsmin-printtrace/ltsmin-printtrace"

set bins [find_alg_backends "{seq,mc,dist,sym}"]
foreach path $bins {
    set bin [lindex [split $path "/"] end]
    set binpaths($bin) $path
}
