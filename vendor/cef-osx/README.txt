Chromium Embedded Framework (CEF) Standard Binary Distribution for Mac OS-X
-------------------------------------------------------------------------------

Date:             September 05, 2015

CEF Version:      3.2454.1320.g807de3c
CEF URL:          https://bitbucket.org/chromiumembedded/cef.git
                  @807de3c161f5598597e40f5a42e8541d9e3eb905

Chromium Verison: 45.0.2454.85
Chromium URL:     https://chromium.googlesource.com/chromium/src.git
                  @4c2743615eaa2806ad014c59bf6acbb652cf3aa8

This distribution contains all components necessary to build and distribute an
application using CEF on the Mac OS-X platform. Please see the LICENSING
section of this document for licensing terms and conditions.


CONTENTS
--------

cefclient   Contains the cefclient sample application configured to build
            using the files in this distribution. This application demonstrates
            a wide range of CEF functionalities.

cefsimple   Contains the cefsimple sample application configured to build
            using the files in this distribution. This application demonstrates
            the minimal functionality required to create a browser window.

Debug       Contains the "Chromium Embedded Framework.framework" and other
            components required to run the debug version of CEF-based
            applications.

include     Contains all required CEF header files.

libcef_dll  Contains the source code for the libcef_dll_wrapper static library
            that all applications using the CEF C++ API must link against.

Release     Contains the "Chromium Embedded Framework.framework" and other
            components required to run the release version of CEF-based
            applications.

tools       Scripts that perform post-processing on Mac release targets.


USAGE
-----

Building using CMake:
  CMake can be used to generate project files in many different formats. See
  usage instructions at the top of the CMakeLists.txt file.

Please visit the CEF Website for additional usage information.

https://bitbucket.org/chromiumembedded/cef/


REDISTRIBUTION
--------------

This binary distribution contains the below components. Components listed under
the "required" section must be redistributed with all applications using CEF.
Components listed under the "optional" section may be excluded if the related
features will not be used.

Applications using CEF on OS X must follow a specific app bundle structure.
Replace "cefclient" in the below example with your application name.

cefclient.app/
  Contents/
    Frameworks/
      Chromium Embedded Framework.framework/
        Chromium Embedded Framework <= main application library
        Resources/
          cef.pak <= non-localized resources and strings
          cef_100_percent.pak <====^
          cef_200_percent.pak <====^
          cef_extensions.pak <=====^
          devtools_resources.pak <=^
          crash_inspector, crash_report_sender <= breakpad support
          icudtl.dat <= unicode support
          natives_blob.bin, snapshot_blob.bin <= V8 initial snapshot
          en.lproj/, ... <= locale-specific resources and strings
          Info.plist
      cefclient Helper.app/
        Contents/
          Info.plist
          MacOS/
            cefclient Helper <= helper executable
          Pkginfo
      cefclient Helper EH.app/
        Contents/
          Info.plist
          MacOS/
            cefclient Helper EH <= helper executable
          Pkginfo
      cefclient Helper NP.app/
        Contents/
          Info.plist
          MacOS/
            cefclient Helper NP <= helper executable
          Pkginfo
      Info.plist
    MacOS/
      cefclient <= cefclient application executable
    Pkginfo
    Resources/
      binding.html, ... <= cefclient application resources

The "Chromium Embedded Framework.framework" is an unversioned framework that
contains CEF binaries and resources. Executables (cefclient, cefclient Helper,
etc) are linked to the "Chromium Embedded Framework" library using
install_name_tool and a path relative to @executable_path.

The "cefclient Helper" apps are used for executing separate processes
(renderer, plugin, etc) with different characteristics. They need to have
separate app bundles and Info.plist files so that, among other things, they
don't show dock icons. The "EH" helper, which is used when launching plugin
processes, has the MH_NO_HEAP_EXECUTION bit cleared to allow an executable
heap. The "NP" helper, which is used when launching NaCl plugin processes
only, has the MH_PIE bit cleared to disable ASLR. This is set up as part of
the build process using scripts from the tools/ directory. Examine the Xcode
project included with the binary distribution or the originating cefclient.gyp
file for a better idea of the script dependencies.

Required components:

The following components are required. CEF will not function without them.

* CEF core library.
  * Chromium Embedded Framework.framework/Chromium Embedded Framework

* Unicode support data.
  * Chromium Embedded Framework.framework/Resources/icudtl.dat

* V8 snapshot data.
  * Chromium Embedded Framework.framework/Resources/natives_blob.bin
  * Chromium Embedded Framework.framework/Resources/snapshot_blob.bin

Optional components:

The following components are optional. If they are missing CEF will continue to
run but any related functionality may become broken or disabled.

* Localized resources.
  Locale file loading can be disabled completely using
  CefSettings.pack_loading_disabled.

  * Chromium Embedded Framework.framework/Resources/*.lproj/
    Directory containing localized resources used by CEF, Chromium and Blink. A
    .pak file is loaded from this directory based on the CefSettings.locale
    value. Only configured locales need to be distributed. If no locale is
    configured the default locale of "en" will be used. Without these files
    arbitrary Web components may display incorrectly.

* Other resources.
  Pack file loading can be disabled completely using
  CefSettings.pack_loading_disabled.

  * Chromium Embedded Framework.framework/Resources/cef.pak
  * Chromium Embedded Framework.framework/Resources/cef_100_percent.pak
  * Chromium Embedded Framework.framework/Resources/cef_200_percent.pak
    These files contain non-localized resources used by CEF, Chromium and Blink.
    Without these files arbitrary Web components may display incorrectly.

  * Chromium Embedded Framework.framework/Resources/cef_extensions.pak
    This file contains non-localized resources required for extension loading.
    Pass the `--disable-extensions` command-line flag to disable use of this
    file. Without this file components that depend on the extension system,
    such as the PDF viewer, will not function.

  * Chromium Embedded Framework.framework/Resources/devtools_resources.pak
    This file contains non-localized resources required for Chrome Developer
    Tools. Without this file Chrome Developer Tools will not function.

* Breakpad support.
  * Chromium Embedded Framework.framework/Resources/crash_inspector
  * Chromium Embedded Framework.framework/Resources/crash_report_sender
  * Chromium Embedded Framework.framework/Resources/Info.plist
    Without these files breakpad support (crash reporting) will not function.


LICENSING
---------

The CEF project is BSD licensed. Please read the LICENSE.txt file included with
this binary distribution for licensing terms and conditions. Other software
included in this distribution is provided under other licenses. Please visit
"about:credits" in a CEF-based application for complete Chromium and third-party
licensing information.
