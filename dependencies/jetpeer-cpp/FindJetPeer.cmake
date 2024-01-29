#
# The jetpeer-cpp library (https://github.com/hbkworld/jetpeer-cpp/) provides two CMake packages,
# named 'jetpeer' and 'jetpeerasync' respectively. This causes problems in a CMake project
# which uses FetchContent and which contains two or more subprojects that both use jetpeer-cpp.
# The reason is that FetchContent's OVERRIDE_FIND_PACKAGE mechanism can only override a single
# dependency name ('jetpeer' or 'jetpeerasync') but not both. The name which is overridden is
# the name passed to FetchContent_*(). It therefore cannot satisfy subsequent find_package()
# calls for both targets.
#
# This function can be used to address this problem. It sets ${JetPeer_FOUND} to 1, if:
#
# - find_package() succeeds for both 'jetpeer' and 'jetpeerasync' (this is the case if
#   jetpeer-cpp is installed locally); OR
# - find_package() succeeds for EITHER 'jetpeer' OR 'jetpeerasync', AND both jet::jetpeer and
#   jet::jetpeerasync are known targets (this is the case if jetpeer-cpp was previously fetched
#   with OVERRIDE_FIND_PACKAGE).
#
# It also sets ${JetPeer_VERSION} to the found or fetched version.
#

function(FindJetPeer JETPEER_MINIMUM_VERSION)

    set(JetPeer_FOUND 0 PARENT_SCOPE)

    find_package(jetpeerasync ${JETPEER_MINIMUM_VERSION} GLOBAL QUIET)
    find_package(jetpeer ${JETPEER_MINIMUM_VERSION} GLOBAL QUIET)

    if(jetpeerasync_FOUND AND jetpeer_FOUND)

        set(JetPeer_VERSION "${jetpeer_VERSION}" PARENT_SCOPE)
        set(JetPeer_FOUND 1 PARENT_SCOPE)

    elseif(jetpeerasync_FOUND OR jetpeer_FOUND)

        if(TARGET jet::jetpeer AND TARGET jet::jetpeerasync)
            if(jetpeerasync_FOUND)
                set(JetPeer_VERSION "${jetpeerasync_VERSION}" PARENT_SCOPE)
            else()
                set(JetPeer_VERSION "${jetpeer_VERSION}" PARENT_SCOPE)
            endif()
            set(JetPeer_FOUND 1 PARENT_SCOPE)
        endif()

    endif()

endfunction()
