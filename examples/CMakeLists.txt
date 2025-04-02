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

set(elora_examples
    elora-cs-example
    elora-tts-example
)

foreach(
  example
  ${elora_examples}
)
  build_lib_example(
    NAME ${example}
    SOURCE_FILES ${example}.cc
    LIBRARIES_TO_LINK
      ${libelora}
      ${libtap-bridge}
      ${libcsma}
  )
endforeach()
