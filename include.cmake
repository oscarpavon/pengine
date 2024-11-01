
#set(OpenGL_GL_PREFERENCE GLVND)

#include(FindFreetype)
#include_directories(${FREETYPE_INCLUDE_DIRS})


#find_package(OpenGL REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

if(CMAKE_SYSTEM_NAME MATCHES "OpenBSD|FreeBSD")

	
endif()


if(CMAKE_SYSTEM_NAME MATCHES "Linux")


set(linux_libraries 
	
  pulse
    pulse-simple
    asound
)


set(pc_link 
	  glfw3
    vulkan
	  ${OPENGL_LIBRARIES}
	  dl
	  m
	  pthread
    EGL 
    GLESv2
    peditor
    freetype
)

set(lib ${lib} ${linux_libraries})

endif()

set(engine_link 
  ${pc_link}
  pavon_engine

    )

set(android_link 
    android
    native_app_glue
    EGL
    GLESv2
    log
    pavon_engine
    m
    peditor
    freetype
    vulkan
    
    )

