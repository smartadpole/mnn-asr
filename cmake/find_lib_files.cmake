function(find_source_file SRC_FILES arg)
    set(file_list "")
    message(STATUS "[*] searching file: ${arg};${ARGN}")

    list(LENGTH ARGV argv_len)
    set(i 1)
    while (i LESS ${argv_len})
        list(GET ARGV ${i} argv_value)
        foreach (file_path ${SRC_DIR})
            file(GLOB file ${file_path}/${argv_value})
            set(file_list ${file_list} ${file})
        endforeach ()

        math(EXPR i "${i} + 1")
    endwhile ()

    set(${SRC_FILES} ${file_list} PARENT_SCOPE)
endfunction()

# add subdirectories to search for source files
set(SRC_DIR
        ${ROOT_DIR}/src
        ${ROOT_DIR}/src/utils
        )