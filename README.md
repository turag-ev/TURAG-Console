# TURAG-Console
## Building
### Windows
- download and unpack latest version of [Qwt](https://qwt.sourceforge.io/)
- to speed up compilation, you can comment out the following entries in qwtconfig.pri:
  - *QWT_CONFIG += QwtDll*
  - *QWT_CONFIG += QwtDesigner*
- follow further steps as outlined in *doc/html/qwtinstall.html*
- create a new environment variable named *QMAKEFEATURES* with the value *C:\Qwt-6.1.2\features* (adjust to your installation path)
- log out and back in to update environment
- open Console.pro in Qt Creator and compile
