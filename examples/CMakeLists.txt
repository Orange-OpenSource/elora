set(base_examples
    simple-network-example
    network-server-example
    complete-network-example
    adr-example
    lorawan-energy-model-example
    aloha-throughput
    parallel-reception-example
    frame-counter-update
    pcap-example
)

foreach(
  example
  ${base_examples}
)
  build_lib_example(
    NAME ${example}
    SOURCE_FILES ${example}.cc
    LIBRARIES_TO_LINK ${libelora}
  )
endforeach()

build_lib_example(
  NAME elora-example
  SOURCE_FILES elora-example.cc
  LIBRARIES_TO_LINK
    ${libelora}
    ${libtap-bridge}
    ${libcsma}
)

build_lib_example(
  NAME elora-example-TTN
  SOURCE_FILES elora-example-TTN.cc
  LIBRARIES_TO_LINK
    ${libelora}
    ${libtap-bridge}
    ${libcsma}
)
