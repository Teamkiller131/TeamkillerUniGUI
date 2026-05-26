function(embed_font TARGET font_file output_header var_name)
    if(WIN32)
        add_custom_command(
            OUTPUT ${output_header}
            COMMAND powershell -NoProfile -ExecutionPolicy Bypass
                -File "${CMAKE_SOURCE_DIR}/cmake/embed_font.ps1"
                -FontFile "${font_file}"
                -OutputHeader "${output_header}"
                -VarName "${var_name}"
            DEPENDS ${font_file} ${CMAKE_SOURCE_DIR}/cmake/embed_font.ps1
            COMMENT "Embedding font: ${font_file}"
        )
    else()
        add_custom_command(
            OUTPUT ${output_header}
            COMMAND xxd -i ${font_file} > ${output_header}
            DEPENDS ${font_file}
            COMMENT "Embedding font: ${font_file} (xxd)"
        )
    endif()
    target_sources(${TARGET} PRIVATE ${output_header})
endfunction()
