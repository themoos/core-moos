include(CheckIncludeFileCXX)
check_include_file_cxx("tr1/unordered_map" HAVE_TR1_UNORDERED_MAP_H)
check_include_file_cxx("unordered_map" HAVE_STD_UNORDERED_MAP_H)


if(HAVE_TR1_UNORDERED_MAP_H)
    add_definitions( -DHAVE_TR1_UNORDERED_MAP )
endif()
if(HAVE_STD_UNORDERED_MAP_H)
    add_definitions( -DHAVE_STD_UNORDERED_MAP )
endif()
