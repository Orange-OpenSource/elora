# ELoRa: An end-to-end LoRaWAN emulator for the ChirpStack network server

>**If you just want to create scenarios using the emulator, and you are not interested in tinkering with it, we refer you to a quicker installation running the [emulator image with Docker Compose](https://github.com/non-det-alle/elora-docker).**

This is a traffic emulator for the [Chirpstack server stack](https://www.chirpstack.io/ "ChirpStack, open-source LoRaWAN® Network Server"). 

This software can be used to simulate in real-time multiple devices and gateways sharing a radio channel with very high flexibility in terms of possible configurations. LoRaWAN traffic is then UDP-encapsulated by gateways and forwarded outside the simulation. If a Chirpstack network server is in place, it will think the traffic is coming from a real network. All Class A MAC primitives used in the UE868 region are supported: radio transmission parameters of simulated devices can be changed by the downlink LoRaWAN traffic of the real server. 

The code is an extension of the ns-3 [LoRaWAN module](https://github.com/signetlabdei/lorawan "LoRaWAN ns-3 module"). In addition to what is provided by the original LoRaWAN module, the following changes/additions were made:

* A gateway application implementing the [UDP forwarder protocol](https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT "Semtech packet forwarder implementation") running on real gateways
* An helper to register devices and gateways in the server using the included [REST API](https://github.com/chirpstack/chirpstack-rest-api "ChirpStack gRPC to REST API proxy")
* Cryptographyc libraries to compute the Meassage Integrity Code (MIC) and encryption of packets for devices to be recognised by the server
* The `elora-example` to show a complete usage of the traffic generator
* Many improvements and corrections of features of the original module, such that traffic could be transparently be accepted by the server

## Prerequisites ##

To use this simulator you need to know the following:

* The ChirpStack server needs to be running somewhere (reachable by the simulation via network)
* The simulator works as is with the default configuration of Chirpstark v4 on `localhost:8080`. It has been tested with the [docker-compose installation](https://www.chirpstack.io/docs/getting-started/docker.html "Chirpstack docs: Quickstart Docker Compose") of the server. To test a distributed version of the setup, the server/port address needs to be changed in `elora-example`, and ChirpStack needs to be set up such that a Gateway Bridge container remains co-located on the same machine of the ELoRa process
* An authentification token needs to be generated in the server (API keys section), and needs to be copy-pasted in `elora-example`

## Installation ##

If not already, install the `libcurl` development library in your linux distribution (`libcurl4-gnutls-dev` on Ubuntu, `curl-dev` on Alpine).

Clone [ns-3](https://www.nsnam.org "ns-3 Website"), checkout the right commit, clone this repository inside the `contrib` directory, and patch ns-3 using the provided [patch file](ns-3-dev.patch) as follows:

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git
cd ns-3-dev/
git checkout ns-3.40
git clone https://github.com/non-det-alle/elora.git contrib/elora
patch -p1 -s < contrib/elora/ns-3-dev.patch
```

Make sure you are in the `ns-3-dev` folder, configure and then build ns-3:

```bash
./ns3 configure -d debug --enable-examples
./ns3 build
```

The `elora` module extends the code of the original `lorawan` ns-3 module, thus the two modules are in conflict if they are built together. If you also have the original `lorawan` module installed (either in the `contrib` or `src` directory), run `./ns3 clean` and add the  `--enable-modules elora` option to the `./ns3 configure` command above to avoid building both.

## Usage examples ##

The module includes the following example:

- `elora-example`

Examples can be run via the `./ns3 run --enable-sudo "elora-example [options]"` command.

Options can be retrived with `./ns3 run "elora-example --help"`.

## Documentation ##

For a description of the module, refer to `doc/lorawan.rst` (currently not up to date with all functionalities).

For more information on how to use the underlying LoRaWAN module refer also to the [original module readme](https://github.com/signetlabdei/lorawan/blob/e8f7a21044418e92759d5c7c4bcab147cdaf05b3/README.md "LoRaWAN ns-3 module README").

- [ns-3 tutorial](https://www.nsnam.org/docs/tutorial/html "ns-3 Tutorial")
- [ns-3 manual](https://www.nsnam.org/docs/manual/html "ns-3 Manual")
- The LoRaWAN specification can be found on the [LoRa Alliance
  website](http://www.lora-alliance.org)

## License ##

This software is licensed under the terms of the GNU GPLv2 (the same license
that is used by ns-3). See the LICENSE.md file for more details.

## Getting help ##

If you need any help, feel free to open an issue here.

## Cite us ##
[A. Aimi, S. Rovedakis, F. Guillemin, and S. Secci, “ELoRa: End-to-end Emulation of Massive IoT LoRaWAN Infrastructures,” 2023 IEEE/IFIP Network Operations and Management Symposium (NOMS), May 2023, Miami, FL, United States.](https://hal.science/hal-04025834)
