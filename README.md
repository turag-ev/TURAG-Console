# TURAG-Console
## Prebuilt Executables
See Releases on the right side.

## Building
### Windows
- download and unpack latest version of [Qwt](https://qwt.sourceforge.io/)
- to speed up compilation, you can comment out the following entries in *qwtconfig.pri*:
  - *QWT_CONFIG += QwtDll*
  - *QWT_CONFIG += QwtDesigner*
- follow further steps as outlined in *doc/html/qwtinstall.html*
- create a new environment variable named *QMAKEFEATURES* with the value *C:\Qwt-6.1.2\features* (adjust to your installation path)
- log out and back in to update environment
- clone repository and update submodules
- open Console.pro in Qt Creator, re-run qmake and compile
- for easy deployment with alle required DLLs the [windeployqt](https://doc.qt.io/qt-6/windows-deployment.html) tool can be used as follows:
  - copy turag_console.exe into an empty release folder
  - open Qt cmd window and change to the bin folder
  - run windeployqt.exe and specify the absolute path to turag_console.exe in the release folder as an argument
  - copy qwt.dll into the release folder
  - run windeployqt.exe again, this time specifying qwt.dll in the release folder as the argument
  
