# SoC_EKF_Linux

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
This repository contains the project on which I based my thesis on. 

The thesis will be published here once it has been discussed.
 
Given the pandemic, no cell could be modeled in a laboratory environment. So I had to use the one given by G.L.Plett in its course on BMS, on Coursera, at the link below:

	https://www.coursera.org/specializations/algorithms-for-battery-management-systems

This repository is structured as follows:

	master
	
	| csv - Contains the csv files of the Cell Model
	
	| logs - Contains the datalogger files
	
	| include - All the libraries used in the project

	| lib - .c files
	
	| Stub - Contains the Stub to test the main project

	|   | include - Contains the procedure library

	|   | lib - Contains the procedure source file


The nomenclature used with the functions in this project is strctured in a way that the first one/two lower case letters of the function represent its return type, and are followed by an upper case letter. 

E.g.:

    pv  pointer to void

    px  pointer

    i   int

    f   float

    l   long

    v   void

    l   long

    u   __u32
 

 To try the application, open two terminals. In the first one type:

    ./Stub/main

And in the second one:

    ./main ./csv/