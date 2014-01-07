Saaghar, a Persian poetry software
----------------------------------------------------------------------------------------
**Copyright (C) 2010-2014, Razi Alavizadeh**

Homepage (in Persian): [http://saaghar.pozh.org](http://saaghar.pozh.org)

My Blog (in Persian): [http://pozh.org](http://pozh.org)

Sourceforge: [http://sourceforge.net/projects/saaghar](http://sourceforge.net/projects/saaghar)

Facebook Page: [https://www.facebook.com/saaghar.p](https://www.facebook.com/saaghar.p)

About Saaghar
----------------------------------------------------------------------------------------
Saaghar is an opensource Persian poetry software, it's based on Qt C++ and it's cross-platform, and available for majaor desktop's operating systems. it  uses "ganjoor.net" database as its database.

**Some of its features:**
* Tabbed UI
* Tabbed and dockable search widgets
* Advanced Search
* Search for Rhymes
* Print and Print Preview
* Export, It supports exporting to "PDF", "HTML", "TeX", "CSV" and "TXT"
  * CSV: indeed is a "Tab Separated Value" version
  * TeX: uses XePersian and bidipoem
  * TXT: is a text document with UTF-8 encoding.
* Copy and Multi-selection
* Icons Theme
* Customisable interface
* Portable Mode

Requirments
----------------------------------------------------------------------------------------
**Windows:**
* To compile and install Saaghar on your system, you have to install the Qt Windows SDK from [http://qt-project.org](http://qt-project.org).

**Linux:**
* To compile and install Saaghar on your system, Be sure you have installed the Qt4 development packages. (maybe qt4-devel, libqt4-dev or similar.)

**Mac OS X:**
* To compile and install Saaghar on your system, you have to install the Qt Mac SDK from [http://qt-project.org](http://qt-project.org) and Mac SDK from 
[http://developer.apple.com](http://developer.apple.com)

--------------------------------------
* Saaghar uses qt sqlite plugins.

Compiling
----------------------------------------------------------------------------------------
    $ cd saaghar
    $ qmake -config release
    $ make
    # make install

Current version
----------------------------------------------------------------------------------------
The current version of Saaghar is **1.0.94**. You can download precompiled packages
and the sources from: [Saaghar's download page](http://pozh.org/saaghar/download).

Development Version
----------------------------------------------------------------------------------------
You can always use the lastest development snapshot of Saaghar from the GIT repository.

**Clone:**

    $ git clone https://github.com/srazi/Saaghar.git

**Download as archive:** [ZIP Archive, Master Branch](https://github.com/srazi/Saaghar/archive/master.zip) 
