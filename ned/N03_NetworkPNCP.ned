simple OutNode03 {
    parameters:
        double timeoutTime = default(1.0);
        int counter = default(10);
        @display("i=icon/chuyin");
    gates:
        output oport;
        output connect;
}

simple InNode03 {
    parameters:
        @display("i=icon/pikachu");
    gates:
        input iport;
        input connect;
}

simple Router03 {
    parameters:
        int queueLength = default(10);
        @display("i=block/routing,white");
    gates:
        input iport[];
        output oport[];
}

network NetworkPNCP {
    types:
        channel Channel extends ned.DelayChannel {
            delay = 100ms;
        }
    submodules:
        outNode[2] : OutNode03;
        inNode[2] : InNode03;
        router : Router03;
    connections:
        outNode[0].oport --> Channel --> router.iport++;
        outNode[1].oport --> Channel --> router.iport++;
        router.oport++ --> Channel --> inNode[0].iport;
        router.oport++ --> Channel --> inNode[1].iport;
        // inter-connection
        outNode[0].connect --> Channel --> inNode[1].connect;
        outNode[1].connect --> Channel --> inNode[0].connect;
}
