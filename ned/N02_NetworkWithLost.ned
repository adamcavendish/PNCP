simple OutNode02 {
    parameters:
        double timeoutTime = default(1.0) @unit(s);
        @display("i=icon/chuyin");
    gates:
        output oport;
}

simple InNode02 {
    parameters:
        @display("i=icon/pikachu");
    gates:
        input iport;
}

simple Router02 {
    parameters:
        @display("i=block/routing,white");
    gates:
        input iport[];
        output oport[];
}

network NetworkWithLost {
    types:
        channel Channel extends ned.DelayChannel {
            delay = 100ms;
        }
    submodules:
        outNode[2] : OutNode02;
        inNode[2] : InNode02;
        router : Router02;
    connections:
        outNode[0].oport --> Channel --> router.iport++;
        outNode[1].oport --> Channel --> router.iport++;
        router.oport++ --> Channel --> inNode[0].iport;
        router.oport++ --> Channel --> inNode[1].iport;
}
