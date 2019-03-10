aux_source_directory(. DIR_SRCS)

add_executable(MavLinkTest ${DIR_SRCS})

target_link_libraries(MavLinkTest MavLinkCom)
