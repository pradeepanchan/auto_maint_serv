Installation:
--------------
This prgram uses Boost Gregorian headers and library for compilation and execution respectively.

Compilation:
-------------
This prgram uses Boost Gregorian headers and library for compilation and execution.
More specifically it uses the cygboost_date_time library

Running the program:
---------------------
 the program will take the following as input:
 1) service guide file
 2) service history file
 3) current odometer reading
 4) prediction of service in kms
 5) output file name
 6) purchase date

 the program will create the following output:
 one file with: parts needed for servicing with service instruction

 service guide file structure is
 part,action,kms,days
 sparkplug,clean,10000,-1
 sparkplug,replace,10000,-1
 engine_oil,replace,5000,6
 
 service history file structure is:
 part,action,kms,date
 sparkplug,replace,15000,30/06/2012
 engine_oil,replace,17000,19/10/2012
 air_filter,clean,17050,19/11/2012
 tappet,inspect,17050,19/11/2012
 
 output file structure is:
 part_action
 engine_oil_replace
 air_filter_clean
