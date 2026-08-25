cmake_minimum_required(VERSION 3.24.0)

##################################
## vcpkg Preamble
include(FetchContent)
FetchContent_Declare(
    vcpkg
    GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
    GIT_TAG        origin/master
)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    FetchContent_Populate(vcpkg)
    set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")
endif()

set(VCPKG_HOST_TRIPLET "${VCPKG_TARGET_TRIPLET}")

##################################
## Project Start
project(krkrz LANGUAGES C CXX ASM_NASM)

add_definitions(/MP)

##################################
## windows7_samples_multimedia_base_classes
add_library(windows7_samples_multimedia_base_classes)

set_target_properties(windows7_samples_multimedia_base_classes
PROPERTIES
    MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_compile_definitions(windows7_samples_multimedia_base_classes
PUBLIC
    _LIB
    UNICODE
    _UNICODE
)

target_sources(windows7_samples_multimedia_base_classes
PRIVATE
    external/baseclasses/amextra.cpp
    external/baseclasses/amextra.h
    external/baseclasses/amfilter.cpp
    external/baseclasses/amfilter.h
    external/baseclasses/amvideo.cpp
    external/baseclasses/arithutil.cpp
    external/baseclasses/cache.h
    external/baseclasses/combase.cpp
    external/baseclasses/combase.h
    external/baseclasses/cprop.cpp
    external/baseclasses/cprop.h
    external/baseclasses/ctlutil.cpp
    external/baseclasses/ctlutil.h
    external/baseclasses/ddmm.cpp
    external/baseclasses/ddmm.h
    external/baseclasses/dllentry.cpp
    external/baseclasses/dllsetup.cpp
    external/baseclasses/dllsetup.h
    external/baseclasses/dxmperf.h
    external/baseclasses/fourcc.h
    external/baseclasses/measure.h
    external/baseclasses/msgthrd.h
    external/baseclasses/mtype.cpp
    external/baseclasses/mtype.h
    external/baseclasses/outputq.cpp
    external/baseclasses/outputq.h
    external/baseclasses/perflog.cpp
    external/baseclasses/perflog.h
    external/baseclasses/perfstruct.h
    external/baseclasses/pstream.cpp
    external/baseclasses/pstream.h
    external/baseclasses/pullpin.cpp
    external/baseclasses/pullpin.h
    external/baseclasses/refclock.cpp
    external/baseclasses/refclock.h
    external/baseclasses/reftime.h
    external/baseclasses/renbase.cpp
    external/baseclasses/renbase.h
    external/baseclasses/schedule.cpp
    external/baseclasses/schedule.h
    external/baseclasses/seekpt.cpp
    external/baseclasses/seekpt.h
    external/baseclasses/source.cpp
    external/baseclasses/source.h
    external/baseclasses/streams.h
    external/baseclasses/strmctl.cpp
    external/baseclasses/strmctl.h
    external/baseclasses/sysclock.cpp
    external/baseclasses/sysclock.h
    external/baseclasses/transfrm.cpp
    external/baseclasses/transfrm.h
    external/baseclasses/transip.cpp
    external/baseclasses/transip.h
    external/baseclasses/videoctl.cpp
    external/baseclasses/videoctl.h
    external/baseclasses/vtrans.cpp
    external/baseclasses/vtrans.h
    external/baseclasses/winctrl.cpp
    external/baseclasses/winctrl.h
    external/baseclasses/winutil.cpp
    external/baseclasses/winutil.h
    external/baseclasses/wxdebug.cpp
    external/baseclasses/wxdebug.h
    external/baseclasses/wxlist.cpp
    external/baseclasses/wxlist.h
    external/baseclasses/wxutil.cpp
    external/baseclasses/wxutil.h
)

target_link_libraries(windows7_samples_multimedia_base_classes
PUBLIC
    strmiids.lib
)

target_include_directories(windows7_samples_multimedia_base_classes 
PUBLIC 
    external/baseclasses
)


##################################
## tvpwin
set(CMAKE_ASM_NASM_COMPILE_OPTIONS "${CMAKE_ASM_NASM_COMPILE_OPTIONS} -safeseh -Xvc -fwin32")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ExecutableName tvpwin64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ExecutableName tvpwin32)
endif()

add_executable(${ExecutableName} WIN32)

set_target_properties(${ExecutableName}
PROPERTIES
    MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_link_options(${ExecutableName}
PUBLIC
    /LARGEADDRESSAWARE
)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(libjpeg-turbo REQUIRED)
find_package(oniguruma CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_path(PICOJSON_INCLUDE_DIRS "picojson/picojson.h" REQUIRED)

# Complex text layout (complete RTL support: Awami Nastaliq etc.)
# Text is shaped through libraqm, which drives HarfBuzz; HarfBuzz must
# have its Graphite2 shaper enabled so fonts carrying Silf tables
# (Awami Nastaliq and other Graphite smart fonts) shape correctly.
find_path(RAQM_INCLUDE_DIR raqm.h REQUIRED)
find_library(RAQM_LIBRARY NAMES raqm REQUIRED)
find_path(HARFBUZZ_INCLUDE_DIR hb.h PATH_SUFFIXES harfbuzz REQUIRED)
find_library(HARFBUZZ_LIBRARY NAMES harfbuzz REQUIRED)
find_library(FREETYPE_LIBRARY NAMES freetype REQUIRED)
find_path(FREETYPE_INCLUDE_DIR ft2build.h PATH_SUFFIXES freetype2 REQUIRED)

include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_INCLUDES ${HARFBUZZ_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIR} ${GRAPHITE2_INCLUDE_DIR})
check_cxx_source_compiles("#include <hb.h>
#include <hb-graphite2.h>
int main() { return (int)(size_t)(void*)&hb_graphite2_face_get_gr_face; }
" KRKRZ_HARFBUZZ_HAS_GRAPHITE2)
unset(CMAKE_REQUIRED_INCLUDES)
unset(CMAKE_REQUIRED_LIBRARIES)
if(NOT KRKRZ_HARFBUZZ_HAS_GRAPHITE2)
message(FATAL_ERROR
    "HarfBuzz was built without Graphite2 support. "
    "Graphite smart fonts such as Awami Nastaliq cannot be shaped without it. "
    "Declare harfbuzz with the graphite2 feature (see vcpkg.json).")
endif()

target_link_libraries(${ExecutableName} 
PRIVATE 
    ZLIB::ZLIB
    PNG::PNG
    freetype
    libjpeg-turbo::jpeg-static
    libjpeg-turbo::turbojpeg-static
    oniguruma::onig
    SDL2::SDL2
    ${RAQM_LIBRARY}
    ${HARFBUZZ_LIBRARY}
    ${FRIBIDI_LIBRARY}
    ${GRAPHITE2_LIBRARY}
)

target_include_directories(${ExecutableName} 
PRIVATE 
    ${JPEG_INCLUDE_DIR}
    ${PICOJSON_INCLUDE_DIRS}
    ${RAQM_INCLUDE_DIR}
    ${HARFBUZZ_INCLUDE_DIR}
    ${FRIBIDI_INCLUDE_DIR}
    ${GRAPHITE2_INCLUDE_DIR}
)

target_compile_definitions(${ExecutableName}
PUBLIC
    __WIN32__
    NO_STRICT
    UNICODE
    _UNICODE
    TJS_TEXT_OUT_CRLF
    TJS_JP_LOCALIZED
    TJS_DEBUG_DUMP_STRING
    _CRT_SECURE_NO_WARNING
    TVP_LOG_TO_COMMANDLINE_CONSOLE
    TVP_REPORT_HW_EXCEPTION
    TVP_ENABLE_EXECUTE_AT_EXCEPTION
    TVP_ENABLE_RAQM
)

message(STATUS "It's here: \n\t${PICOJSON_INCLUDE_DIRS}\n\t${JPEG_INCLUDE_DIR}\n\t${JPEG_LIBRARIES}")

target_include_directories(${ExecutableName}
PRIVATE
    external/baseclasses
    movie/win32
    visual/gl
    extension
    tjs2
    base
    base/win32
    ./
    msg
    msg/win32
    sound
    sound/win32
    utils
    utils/win32
    visual
    visual/win32
    environ/win32
    visual/IA32
    vcproj
    platform/win32
)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_sources(${ExecutableName} 
    PRIVATE
        visual/IA32/tlg6_chroma.nas
        visual/IA32/tlg6_golomb.nas
    )
    set_source_files_properties(visual/IA32/tlg6_chroma.nas PROPERTIES LANGUAGE ASM_NASM)
    set_source_files_properties(visual/IA32/tlg6_golomb.nas PROPERTIES LANGUAGE ASM_NASM)
endif()

target_sources(${ExecutableName} 
PRIVATE
    vcproj/tvpwin32.rc
    vcproj/dpi.manifest

    base/BinaryStream.cpp
    base/BinaryStream.h
    base/CharacterSet.cpp
    base/CharacterSet.h
    base/EventIntf.cpp
    base/EventIntf.h
    base/PluginIntf.cpp
    base/PluginIntf.h
    base/ScriptMgnIntf.cpp
    base/ScriptMgnIntf.h
    base/StorageIntf.cpp
    base/StorageIntf.h
    base/SysInitIntf.cpp
    base/SysInitIntf.h
    base/SystemIntf.cpp
    base/SystemIntf.h
    base/TextStream.cpp
    base/TextStream.h
    base/UtilStreams.cpp
    base/UtilStreams.h
    base/win32/EventImpl.cpp
    base/win32/EventImpl.h
    base/win32/FileSelector.cpp
    base/win32/FileSelector.h
    base/win32/FuncStubs.cpp
    base/win32/FuncStubs.h
    base/win32/NativeEventQueue.cpp
    base/win32/NativeEventQueue.h
    base/win32/PluginImpl.cpp
    base/win32/PluginImpl.h
    base/win32/ScriptMgnImpl.cpp
    base/win32/StorageImpl.cpp
    base/win32/StorageImpl.h
    base/win32/SusieArchive.cpp
    base/win32/SusieArchive.h
    base/win32/SysInitImpl.cpp
    base/win32/SysInitImpl.h
    base/win32/SystemImpl.cpp
    base/win32/SystemImpl.h
    base/XP3Archive.cpp
    base/XP3Archive.h
    environ/win32/Application.cpp
    environ/win32/Application.h
    environ/win32/ApplicationSpecialPath.h
    environ/win32/CompatibleNativeFuncs.cpp
    environ/win32/CompatibleNativeFuncs.h
    environ/win32/ConfigFormUnit.cpp
    environ/win32/ConfigFormUnit.h
    environ/win32/DetectCPU.cpp
    environ/win32/DetectCPU.h
    environ/win32/EmergencyExit.cpp
    environ/win32/EmergencyExit.h
    environ/win32/MouseCursor.cpp
    environ/win32/MouseCursor.h
    environ/win32/SystemControl.cpp
    environ/win32/SystemControl.h
    environ/win32/TouchPoint.cpp
    environ/win32/TouchPoint.h
    environ/win32/TVPWindow.cpp
    environ/win32/TVPWindow.h
    environ/win32/VersionFormUnit.cpp
    environ/win32/VersionFormUnit.h
    environ/win32/WindowFormUnit.cpp
    environ/win32/WindowFormUnit.h
    environ/win32/WindowsUtil.cpp
    environ/win32/WindowsUtil.h
    extension/Extension.cpp
    extension/Extension.h
    movie/win32/asyncio.cpp
    movie/win32/asyncio.h
    movie/win32/asyncrdr.cpp
    movie/win32/asyncrdr.h
    movie/win32/BufferRenderer.cpp
    movie/win32/BufferRenderer.h
    movie/win32/CDemuxOutputPin.cpp
    movie/win32/CDemuxOutputPin.h
    movie/win32/CDemuxSource.cpp
    movie/win32/CDemuxSource.h
    movie/win32/CDLLLoader.h
    movie/win32/CIStream.h
    movie/win32/CMediaSeekingProxy.cpp
    movie/win32/CMediaSeekingProxy.h
    movie/win32/CVMRCustomAllocatorPresenter9.cpp
    movie/win32/CVMRCustomAllocatorPresenter9.h
    movie/win32/CWMAllocator.cpp
    movie/win32/CWMAllocator.h
    movie/win32/CWMBuffer.cpp
    movie/win32/CWMBuffer.h
    movie/win32/CWMReader.cpp
    movie/win32/CWMReader.h
    movie/win32/DShowException.cpp
    movie/win32/DShowException.h
    movie/win32/dslayerd.cpp
    movie/win32/dslayerd.h
    movie/win32/dsmixer.cpp
    movie/win32/dsmixer.h
    movie/win32/dsmovie.cpp
    movie/win32/dsmovie.h
    movie/win32/dsoverlay.cpp
    movie/win32/dsoverlay.h
    movie/win32/IBufferRenderer.h
    movie/win32/IBufferRenderer_i.c
    movie/win32/IDemuxReader.h
    movie/win32/IRendererBufferAccess.h
    movie/win32/IRendererBufferAccess_i.c
    movie/win32/IRendererBufferVideo.h
    movie/win32/IRendererBufferVideo_i.c
    movie/win32/krlmovie.cpp
    movie/win32/krmmovie.cpp
    movie/win32/krmovie.cpp
    movie/win32/MFPlayer.cpp
    movie/win32/MFPlayer.h
    movie/win32/OptionInfo.h
    movie/win32/PlayWindow.cpp
    movie/win32/PlayWindow.h
    movie/win32/TVPVideoOverlay.cpp
    movie/win32/TVPVideoOverlay.h
    msg/MsgIntf.cpp
    msg/MsgIntf.h
    msg/win32/MsgImpl.cpp
    msg/win32/MsgImpl.h
    msg/win32/MsgLoad.cpp
    msg/win32/ReadOptionDesc.cpp
    msg/win32/ReadOptionDesc.h
    #msg/svn_revision.h
    sound/MathAlgorithms.cpp
    sound/MathAlgorithms.h
    sound/MathAlgorithms_SSE.cpp
    sound/PhaseVocoderDSP.cpp
    sound/PhaseVocoderDSP.h
    sound/PhaseVocoderFilter.cpp
    sound/PhaseVocoderFilter.h
    sound/RealFFT.cpp
    sound/RealFFT.h
    sound/RealFFT_SSE.cpp
    sound/RingBuffer.h
    sound/SoundBufferBaseIntf.cpp
    sound/SoundBufferBaseIntf.h
    sound/WaveFormatConverter.cpp
    sound/WaveFormatConverter_SSE.cpp
    sound/WaveIntf.cpp
    sound/WaveIntf.h
    sound/WaveLoopManager.cpp
    sound/WaveLoopManager.h
    sound/WaveSegmentQueue.cpp
    sound/WaveSegmentQueue.h
    sound/win32/kmp_pi.h
    sound/win32/oldwaveunpacker.h
    sound/win32/SoundBufferBaseImpl.cpp
    sound/win32/SoundBufferBaseImpl.h
    sound/win32/tvpsnd.c
    sound/win32/tvpsnd.h
    sound/win32/WaveImpl.cpp
    sound/win32/WaveImpl.h
    sound/xmmlib.cpp
    sound/xmmlib.h
    tjs2/tjs.cpp
    tjs2/tjs.h
    tjs2/tjs.tab.cpp
    tjs2/tjs.tab.hpp
    tjs2/tjsArray.cpp
    tjs2/tjsArray.h
    tjs2/tjsBinarySerializer.cpp
    tjs2/tjsBinarySerializer.h
    tjs2/tjsByteCodeLoader.cpp
    tjs2/tjsByteCodeLoader.h
    tjs2/tjsCommHead.h
    tjs2/tjsCompileControl.cpp
    tjs2/tjsCompileControl.h
    tjs2/tjsConfig.cpp
    tjs2/tjsConfig.h
    tjs2/tjsConstArrayData.cpp
    tjs2/tjsConstArrayData.h
    tjs2/tjsDate.cpp
    tjs2/tjsDate.h
    tjs2/tjsdate.tab.cpp
    tjs2/tjsdate.tab.hpp
    tjs2/tjsDateParser.cpp
    tjs2/tjsDateParser.h
    tjs2/tjsDebug.cpp
    tjs2/tjsDebug.h
    tjs2/tjsDictionary.cpp
    tjs2/tjsDictionary.h
    tjs2/tjsDisassemble.cpp
    tjs2/tjsError.cpp
    tjs2/tjsError.h
    tjs2/tjsErrorDefs.h
    tjs2/tjsException.cpp
    tjs2/tjsException.h
    tjs2/tjsGlobalStringMap.cpp
    tjs2/tjsGlobalStringMap.h
    tjs2/tjsHashSearch.h
    tjs2/tjsInterCodeExec.cpp
    tjs2/tjsInterCodeExec.h
    tjs2/tjsInterCodeGen.cpp
    tjs2/tjsInterCodeGen.h
    tjs2/tjsInterface.cpp
    tjs2/tjsInterface.h
    tjs2/tjsLex.cpp
    tjs2/tjsLex.h
    tjs2/tjsMath.cpp
    tjs2/tjsMath.h
    tjs2/tjsMessage.cpp
    tjs2/tjsMessage.h
    tjs2/tjsMT19937ar-cok.cpp
    tjs2/tjsMT19937ar-cok.h
    tjs2/tjsNamespace.cpp
    tjs2/tjsNamespace.h
    tjs2/tjsNative.cpp
    tjs2/tjsNative.h
    tjs2/tjsObject.cpp
    tjs2/tjsObject.h
    tjs2/tjsObjectExtendable.cpp
    tjs2/tjsObjectExtendable.h
    tjs2/tjsOctPack.cpp
    tjs2/tjsOctPack.h
    tjs2/tjspp.tab.cpp
    tjs2/tjspp.tab.hpp
    tjs2/tjsRandomGenerator.cpp
    tjs2/tjsRandomGenerator.h
    tjs2/tjsRegExp.cpp
    tjs2/tjsRegExp.h
    tjs2/tjsScriptBlock.cpp
    tjs2/tjsScriptBlock.h
    tjs2/tjsScriptCache.cpp
    tjs2/tjsScriptCache.h
    tjs2/tjsString.cpp
    tjs2/tjsString.h
    tjs2/tjsTypes.h
    tjs2/tjsUtils.cpp
    tjs2/tjsUtils.h
    tjs2/tjsVariant.cpp
    tjs2/tjsVariant.h
    tjs2/tjsVariantString.cpp
    tjs2/tjsVariantString.h
    utils/ClipboardIntf.cpp
    utils/ClipboardIntf.h
    utils/cp932_uni.cpp
    utils/cp932_uni.h
    utils/Debugger.h
    utils/DebugIntf.cpp
    utils/DebugIntf.h
    utils/Exception.h
    utils/FilePathUtil.h
    utils/md5.c
    utils/md5.h
    utils/MiscUtility.cpp
    utils/ObjectList.h
    utils/Random.cpp
    utils/Random.h
    utils/StringUtil.h
    utils/ThreadIntf.cpp
    utils/ThreadIntf.h
    utils/TickCount.cpp
    utils/TickCount.h
    utils/TimerImpl.cpp
    utils/TimerImpl.h
    utils/TimerIntf.cpp
    utils/TimerIntf.h
    utils/uni_cp932.cpp
    utils/uni_cp932.h
    utils/VelocityTracker.cpp
    utils/VelocityTracker.h
    utils/win32/ClipboardImpl.cpp
    utils/win32/ThreadImpl.cpp
    utils/win32/ThreadImpl.h
    utils/win32/TVPTimer.cpp
    utils/win32/TVPTimer.h
    vcproj/Resource.h
    visual/argb.h
    visual/BitmapIntf.cpp
    visual/BitmapIntf.h
    visual/BitmapLayerTreeOwner.cpp
    visual/BitmapLayerTreeOwner.h
    visual/CharacterData.cpp
    visual/CharacterData.h
    visual/ComplexRect.cpp
    visual/ComplexRect.h
    visual/drawable.h
    visual/FontRasterizer.h
    visual/FontSystem.cpp
    visual/FontSystem.h
    visual/FreeType.cpp
    visual/FreeType.h
    visual/FreeTypeFace.h
    visual/FreeTypeFontRasterizer.cpp
    visual/FreeTypeFontRasterizer.h
    visual/gl/adjust_color_sse2.cpp
    visual/gl/aligned_allocator.h
    visual/gl/blend_function.cpp
    visual/gl/blend_function_avx2.cpp
    visual/gl/blend_function_sse2.cpp
    visual/gl/blend_functor_avx2.h
    visual/gl/blend_functor_c.h
    visual/gl/blend_functor_sse2.h
    visual/gl/blend_ps_functor_sse2.h
    visual/gl/blend_util_func.h
    visual/gl/blend_variation.h
    visual/gl/boxblur_sse2.cpp
    visual/gl/colorfill_sse2.cpp
    visual/gl/colormap_sse2.cpp
    visual/gl/interpolation_functor_sse2.h
    visual/gl/pixelformat_sse2.cpp
    visual/gl/ResampleImage.cpp
    visual/gl/ResampleImage.h
    visual/gl/ResampleImageAVX2.cpp
    visual/gl/ResampleImageInternal.h
    visual/gl/ResampleImageSSE2.cpp
    visual/gl/simd_def_x86x64.h
    visual/gl/sse_mathfun.h
    visual/gl/tlg_sse2.cpp
    visual/gl/univtrans_sse2.cpp
    visual/gl/WeightFunctor.cpp
    visual/gl/WeightFunctor.h
    visual/gl/WeightFunctorAVX.h
    visual/gl/WeightFunctorSSE.h
    visual/gl/x86simdutil.cpp
    visual/gl/x86simdutil.h
    visual/gl/x86simdutilAVX2.cpp
    visual/GraphicsLoaderIntf.cpp
    visual/GraphicsLoaderIntf.h
    visual/GraphicsLoadThread.cpp
    visual/GraphicsLoadThread.h
    visual/IA32/detect_cpu.cpp
    visual/IA32/tvpgl_ia32_intf.c
    visual/IA32/tvpgl_ia32_intf.h
    visual/ImageFunction.cpp
    visual/ImageFunction.h
    visual/LayerBitmapImpl.cpp
    visual/LayerBitmapImpl.h
    visual/LayerBitmapIntf.cpp
    visual/LayerBitmapIntf.h
    visual/LayerIntf.cpp
    visual/LayerIntf.h
    visual/LayerManager.cpp
    visual/LayerManager.h
    visual/LayerTreeOwnerImpl.cpp
    visual/LayerTreeOwnerImpl.h
    visual/LoadJPEG.cpp
    visual/LoadJXR.cpp
    visual/LoadPNG.cpp
    visual/LoadTLG.cpp
    #visual/LoadTLG.h
    visual/PrerenderedFont.cpp
    visual/PrerenderedFont.h
    visual/RectItf.cpp
    visual/RectItf.h
    visual/SaveTLG.h
    visual/SaveTLG5.cpp
    visual/SaveTLG6.cpp
    visual/transhandler.h
    visual/TransIntf.cpp
    visual/TransIntf.h
    visual/tvpfontstruc.h
    visual/tvpgl.c
    visual/tvpgl.h
    visual/tvpinputdefs.h
    visual/VideoOvlIntf.cpp
    visual/VideoOvlIntf.h
    visual/voMode.h
    visual/win32/BasicDrawDevice.cpp
    visual/win32/BasicDrawDevice.h
    visual/win32/BitmapBitsAlloc.cpp
    visual/win32/BitmapBitsAlloc.h
    visual/win32/BitmapInfomation.cpp
    visual/win32/BitmapInfomation.h
    visual/win32/DInputMgn.cpp
    visual/win32/DInputMgn.h
    visual/win32/DrawDevice.cpp
    visual/win32/DrawDevice.h
    visual/win32/GDIFontRasterizer.cpp
    visual/win32/GDIFontRasterizer.h
    visual/win32/GraphicsLoaderImpl.cpp
    visual/win32/GraphicsLoaderImpl.h
    visual/win32/krmovie.h
    visual/win32/LayerImpl.cpp
    visual/win32/LayerImpl.h
    visual/win32/NativeFreeTypeFace.cpp
    visual/win32/NativeFreeTypeFace.h
    visual/win32/SDLInputMgr.cpp
    visual/win32/SDLInputMgr.h
    visual/win32/TVPColor.h
    visual/win32/TVPScreen.cpp
    visual/win32/TVPScreen.h
    visual/win32/TVPSysFont.cpp
    visual/win32/TVPSysFont.h
    visual/win32/VideoOvlImpl.cpp
    visual/win32/VideoOvlImpl.h
    visual/win32/VSyncTimingThread.cpp
    visual/win32/VSyncTimingThread.h
    visual/win32/WindowImpl.cpp
    visual/win32/WindowImpl.h
    visual/WindowIntf.cpp
    visual/WindowIntf.h
)

set_source_files_properties(
    visual/gl/blend_function_avx2.cpp
    visual/gl/ResampleImageAVX2.cpp
    visual/gl/x86simdutilAVX2.cpp

PROPERTIES 
    COMPILE_OPTIONS 
        /arch:AVX2
)

target_link_libraries(${ExecutableName}
PRIVATE
    kernel32.lib
    user32.lib
    gdi32.lib
    winspool.lib
    comdlg32.lib
    advapi32.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    odbc32.lib
    odbccp32.lib
    winmm.lib
    dsound.lib
    version.lib
    mpr.lib
    shlwapi.lib
    vfw32.lib
    imm32.lib
    windows7_samples_multimedia_base_classes
    setupapi.lib
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ExecutableName}) 