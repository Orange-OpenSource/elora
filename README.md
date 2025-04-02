# ELoRa: An end-to-end emulator for LoRaWAN server stacks

>**If you just to try it out, we refer you to a quicker installation running the [emulator image with Docker Compose](https://github.com/non-det-alle/elora-docker).**

This is an in-depth traffic emulator for LoRaWAN traffic compatible with both [ChirpStack](https://www.chirpstack.io/ "ChirpStack, open-source LoRaWAN® Network Server") and [The Things Stack](https://www.thethingsindustries.com/docs/, "The Things Stack Documentation"), the most widespread open-source implementation of a real LoRaWAN server stack.

This software can be used to simulate in real-time multiple devices and gateways sharing a radio channel with very high flexibility in terms of possible configurations. LoRaWAN traffic is then UDP-encapsulated by gateways and forwarded outside the simulation. If a network server is present, it will think the traffic is coming from a real network. All Class A MAC primitives used in the UE868 region are supported, so **radio transmission parameters of simulated devices are affected by the downlink LoRaWAN traffic of the real server**.

The code is an extension of the ns-3 [LoRaWAN module](https://github.com/signetlabdei/lorawan "LoRaWAN ns-3 module"). In addition to what is provided by the original LoRaWAN module, the following changes/additions were made:

* A gateway application implementing the [UDP forwarder protocol](https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT "Semtech packet forwarder implementation") running on real gateways
* An helper to register devices and gateways in the servers using their REST APIs [[1](https://github.com/chirpstack/chirpstack/tree/master/api/proto/api "ChirpStack REST API definitions in proto files"), [2](https://www.thethingsindustries.com/docs/api/reference/http/, "The THings Stack REST API docs")]
* Cryptographyc libraries to compute the Meassage Integrity Code (MIC) and encryption of packets for devices to be recognised by the server
* The `elora-cs-example` and `elora-ttn-example` to show a complete usage of the traffic generator
* Many improvements and corrections of features of the original module, such that traffic could be transparently be accepted by the server

## Prerequisites

To use this simulator you need to know the following:

* The ChirpStack or The Things Stack server needs to be running somewhere (reachable by the simulation via network)
* The ChirpStack example (`elora-cs-example`) works as it is with the default configuration of Chirpstark v4.9.0 on `localhost:8080`. It has been tested with the [docker-compose installation](https://www.chirpstack.io/docs/getting-started/docker.html "ChirpStack docs: Quickstart Docker Compose") of the server. To test a distributed version of the setup, the server/port address needs to be changed in `elora-cs-example`, and ChirpStack needs to be set up such that a Gateway Bridge container remains co-located on the same machine of the ELoRa process
* The Things Stack example (`elora-ttn-example`) works as it is with the default configuration of The Things Stack 3.33.1 on `localhost:8885`. It has been tested with the [docker-compose installation](https://www.thethingsindustries.com/docs/enterprise/docker/ "The Things Stack docs: Docker") of the server. It is a bit more complicated but you can follow an official [video](https://www.thethingsindustries.com/docs/enterprise/docker/configuration/#running-the-things-stack-as-localhost) to reproduce the installation.
* An authentification token needs to be generated in the correct server (API keys section), and needs to be copy-pasted in `elora-cs-example` or `elora-tts-example`.

## Installation

If not already, install the `libcurl` development library in your linux distribution (`libcurl4-gnutls-dev` on Ubuntu, `curl-dev` on Alpine).

Clone [ns-3](https://www.nsnam.org "ns-3 Website"), clone this repository inside the `contrib` directory, and checkout the right commit with the following all-in-one command:

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git && cd ns-3-dev &&
git clone https://github.com/Orange-OpenSource/elora.git contrib/elora &&
tag=$(< contrib/elora/NS3-VERSION) && tag=${tag#release } && git checkout $tag -b $tag
```

Make sure you are in the `ns-3-dev` directory, then configure and build ns-3 with the following all-in-one command:

```bash
./ns3 configure -d debug --enable-examples &&
./ns3 build
```

The `elora` module extends the code of the original `lorawan` ns-3 module, thus the two modules are in conflict if they are built together. If you also have the original `lorawan` module installed (either in the `contrib` or `src` directory), run `./ns3 clean` and add the  `--enable-modules "elora;tap-bridge;csma"` option to the `./ns3 configure` command above to avoid building both.

## Usage examples

The module includes the following examples:

* `elora-cs-example`
* `elora-ttn-example`

Examples can be run via the `./ns3 run --enable-sudo "elora-example [options]"` command.

Options can be retrived with `./ns3 run "elora-example --help"`.

## Documentation

For a description of the module, refer to `doc/lorawan.rst` (currently not up to date with all functionalities).

For more information on how to use the underlying LoRaWAN module refer also to the [original ns-3 repository](https://github.com/signetlabdei/lorawan "LoRaWAN ns-3 module").

* [ns-3 tutorial](https://www.nsnam.org/docs/tutorial/html "ns-3 Tutorial")
* [ns-3 manual](https://www.nsnam.org/docs/manual/html "ns-3 Manual")
* The LoRaWAN specification can be found on the [LoRa Alliance
  website](http://www.lora-alliance.org)

## Copyright

Code copyright 2022-2023 Orange.

## License

This software is licensed under the terms of the GNU GPLv2 (the same license
that is used by ns-3). See the LICENSE.md file for more details.

## Getting help

If you need any help, feel free to open an issue here.

## Publications

A. Aimi, S. Rovedakis, F. Guillemin and S. Secci, "ELoRa: End-to-end Emulation of Massive IoT LoRaWAN Infrastructures," NOMS 2023-2023 IEEE/IFIP Network Operations and Management Symposium, Miami, FL, USA, 2023, pp. 1-3, doi: [10.1109/NOMS56928.2023.10154373](https://doi.org/10.1109/NOMS56928.2023.10154373).

### Contributors

* C. F. Hernandez, O. Iova and F. Valois, "Downlink Scheduling in LoRaWAN: ChirpStack vs The Things Stack," 2024 IEEE Latin-American Conference on Communications (LATINCOM), Medellin, Colombia, 2024, pp. 1-6, doi: [10.1109/LATINCOM62985.2024.10770676](https://doi.org/10.1109/LATINCOM62985.2024.10770676).
