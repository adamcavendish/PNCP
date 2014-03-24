simple OutNode01 {
    parameters:
        @display("i=icon/chuyin");
    gates:
        output oport;
}

simple InNode01 {
    parameters:
        @display("i=icon/pikachu");
    gates:
        input iport;
}

simple Router01 {
    parameters:
        @display("i=block/routing,white");
    gates:
        input iport[];
        output oport[];
}

network NetworkRaw {
    types:
        channel Channel extends ned.DelayChannel {
            delay = 100ms;
        }
    submodules:
        outNode[2] : OutNode01;
        inNode[2] : InNode01;
        router : Router01;
    connections:
        outNode[0].oport --> Channel --> router.iport++;
        outNode[1].oport --> Channel --> router.iport++;
        router.oport++ --> Channel --> inNode[0].iport;
        router.oport++ --> Channel --> inNode[1].iport;
}
