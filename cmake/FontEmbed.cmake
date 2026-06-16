function(embed_font TARGET font_file output_header var_name)
    if(WIN32)
        add_custom_command(
            OUTPUT ${output_header}
            COMMAND powershell -NoProfile -ExecutionPolicy Bypass
                -File "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed_font.ps1"
                -FontFile "${font_file}"
                -OutputHeader "${output_header}"
                -VarName "${var_name}"
            DEPENDS ${font_file} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed_font.ps1
            COMMENT "Embedding font: ${font_file}"
        )
    else()
        add_custom_command(
            OUTPUT ${output_header}
            COMMAND python3 "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed_font.py" "${font_file}" "${output_header}" "${var_name}"
            DEPENDS ${font_file} "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed_font.py"
            COMMENT "Embedding font: ${font_file}"
        )
    endif()
    target_sources(${TARGET} PRIVATE ${output_header})
endfunction()
