simple OutNode04 {
    parameters:
        int counter = default(10);
        double generatePacketProb = default(1.0); // 0.0 - 1.0 being valid.
        string logPath = default("log/OutNode04.log");
        @display("i=icon/chuyin");
    gates:
        output oport;
        output connect;
}

simple InNode04 {
    parameters:
        @display("i=icon/pikachu");
        string logPath = default("log/InNode04.log");
    gates:
        input iport;
        input connect;
}

simple Router04 {
    parameters:
        int queueLength = default(10);
        double alphaProb = default(0.5); // 0.0 - 1.0 being valid
        double betaProb = default(0.5); // 0.0 - 1.0 being valid
        string logPath = default("log/Router04.log");
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
        outNode[2] : OutNode04;
        inNode[2] : InNode04;
        router : Router04;
    connections:
        outNode[0].oport --> Channel --> router.iport++;
        outNode[1].oport --> Channel --> router.iport++;
        router.oport++ --> Channel --> inNode[0].iport;
        router.oport++ --> Channel --> inNode[1].iport;
        // inter-connection
        outNode[0].connect --> Channel --> inNode[1].connect;
        outNode[1].connect --> Channel --> inNode[0].connect;
}

