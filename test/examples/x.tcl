# tcl tests

#simple example

proc Echo_Server {port} {
    set s [socket -server EchoAccept $port]
    vwait forever
}
