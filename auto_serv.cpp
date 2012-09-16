//Name: auto_serv
//Description: This program has been developed to help you to guide you to decide on which items you need to service in your next upcoming service date for your vehicle

/*
	Copyright (c) 2012, Pradeepkumar B. Anchan

    This program is an open source software: you can redistribute it and/or modify
    it under the terms of the ISC License as published by
    the Open Source Initiative (OSI) or any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    ISC License for more details.

    You should have received a copy of the ISC License along with this program,
	in a file called LICENSE. If not, see <http://opensource.org/licenses/ISC>.
*/

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <sstream>
#include <tuple>
#include <forward_list>

using namespace std;
using namespace boost::gregorian;

typedef unsigned long DistanceInKms; 
typedef unsigned long Days; 
typedef string PartName; 
typedef string Action; 
typedef string Part_Action; 
typedef tuple<PartName, Action, DistanceInKms, date> PartDetailInDate;
typedef map<Part_Action, tuple<DistanceInKms, date> >PartsInfoInDate;
typedef tuple<PartName, Action, DistanceInKms, Days> PartDetailInDays;
typedef map<Part_Action, tuple<DistanceInKms, Days> > PartsInfoInDays;


const string serviceGuideFileName;	// used to store the name of the file which will contain
									// the service-guide of the automobile
const string serviceHistoryFileName;	// used to store the name of the file which will contain
										// the service-history of the automobile
const string nextServiceItemsFileName;	// used to store the file-name which will contain
										// the next items to service

//================================================================================================
class FileStream : public fstream
{
	public:
		FileStream( const char * filename, ios_base::openmode mode );
		~FileStream();
	
	private:
		FileStream() {};
		void open( const char * filename, ios_base::openmode mode );
		void close();
};

FileStream::FileStream( const char * filename, ios_base::openmode mode = ios_base::in | ios_base::out )
	: fstream( filename , mode )
{
	if ( ios::fail() )
	{
		stringstream message;
		message << "file open error for file <" << filename << ">" << endl;
		throw runtime_error(message.str());
	}
	exceptions( ios_base::failbit | ios_base::badbit );
}

FileStream::~FileStream()
{
	fstream::close();
}

//================================================================================================

class StringTokeniser
{
	public:
		static deque<string> parse( const string & i_string , char i_delim );
	private:
		StringTokeniser(void) {};
		~StringTokeniser() {};
};

deque<string> StringTokeniser::parse( const string & i_string , char i_delim )
{
	deque<string> o_strings;

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ << " at location: before string size check" << endl;
    cout << "===================================" << endl;
#endif

	if ( ! (i_string.size()) ) throw logic_error("empty string passed for parsing");

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ << " at location: after string size check" << endl;
    cout << "===================================" << endl;
#endif

#ifdef __DEBUG
	cout << "===================================" << endl;
	cout << "From function: " << __FUNCTION__ << " input string:" << i_string << endl;
	cout << "===================================" << endl;
#endif

 	size_t delimPos = i_string.find_first_of(i_delim);

#ifdef __DEBUG
	cout << "===================================" << endl;
	cout << "From function: " << __FUNCTION__ << " first delimiter position:" << delimPos << endl;
	cout << "===================================" << endl;
#endif

	o_strings.push_back(i_string.substr( 0 , delimPos ));

	size_t nextDelimPos;
	while ( ( nextDelimPos = i_string.find_first_of(i_delim, delimPos+1) ) != string::npos )
	{

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ << " next delimiter position:" << nextDelimPos << endl;
    cout << "===================================" << endl;
#endif

		o_strings.push_back(i_string.substr( delimPos+1 , nextDelimPos-delimPos-1 ));
		delimPos = nextDelimPos;
	}

#ifdef __DEBUG
	cout << "===================================" << endl;
	cout << "From function: " << __FUNCTION__ << " strings parsed:" << endl;
	for ( auto ptr = o_strings.begin() ; ptr != o_strings.end() ; ptr++ )
	{
		cout << *ptr << endl;
	}
	cout << "===================================" << endl;
#endif

	return o_strings;
}

//================================================================================================

class CLIParser
{
	public:
		CLIParser(int argc, char *argv[]);
		string getServiceGuideFileName() const { return m_serviceGuideFileName; }
		string getServiceHistoryFileName() const { return m_serviceHistoryFileName; }
		string getNextServiceItemsFileName() const { return m_nextServiceItemsFileName; }
		DistanceInKms getCurrentOdometer() const { return m_current_odometer; }
		DistanceInKms getPredictionKms() const { return m_prediction_kms; }
		date get_purchase_date() const { return m_purchase_date; }
	private:
		void parseArgs(int argc, char *argv[]);
		void validateInput(int argc, char *argv[]);
		void printUsage() const;
		CLIParser();
		CLIParser(const CLIParser &);
		const string m_serviceGuideFileName;
		const string m_serviceHistoryFileName;
		const string m_nextServiceItemsFileName;
		const DistanceInKms m_current_odometer;
		const DistanceInKms m_prediction_kms;
		const date m_purchase_date;
};

CLIParser::CLIParser( int argc, char *argv[] )
:		m_serviceGuideFileName(""),
		m_serviceHistoryFileName(""),
		m_nextServiceItemsFileName(""),
		m_current_odometer(0),
		m_prediction_kms(0),
		m_purchase_date(1970,Jan,1)
{
	try
	{
		validateInput(argc, argv);
		parseArgs(argc, argv);
	}
	catch ( logic_error &e )
	{
		cerr << "Caught an error in handling command line parameters:\n"
			"error message: " << e.what() << endl ;
		printUsage();
		throw;
	}
}

void CLIParser::printUsage() const
{
	cerr <<	endl << endl <<
		"input arguments should be as follows:\n"
		"auto_serv_help -g service_guide_file_name -h service_history -n next_service_items -o current_odometer -p number_of_kms_to_predict -P purchase_date\n"
		"where:\n"
				"\t\t -g service_guide_file_name : the name of the file which contains the service "
								"guide of your automobile\n"
				"\t\t -h service_history : the name of the file which contains the service history "
								"of your automobile\n"
				"\t\t -o kms : current odometer reading of the vehicle\n"
				"\t\t -p prediction_kms: the number of kms for which you want to foresee any part "
								"replacement (useful for touring)\n"
				"\t\t -P purchase_date: the date the vehicle was purchased from the showroom "
								"(if you are not sure you can put some very old date like 5 years "
								"back)\n"
				"\t\t -n next_service_items : the name of the file which the process will create "
								"which will contain the next service request "
								"for your workshop" << endl << endl << endl;
}

void CLIParser::validateInput(int argc, char *argv[])
{
	if ( argc < 13 )
	{
		throw logic_error("Invalid number of inputs");
	}
}

void CLIParser::parseArgs(int argc, char *argv[])
{
	for (int iter = 1 // starting from 1 because the first input argument: iter=0, is the exe-name 
			; iter < argc ; iter++ )
	{
		if ( argv[iter][0] == '-' )
		{
			switch ( argv[iter][1] )
			{
				case 'g' : const_cast<string &>(m_serviceGuideFileName) = argv[++iter];
							break;
				case 'h' : const_cast<string &>(m_serviceHistoryFileName) = argv[++iter];
							break;
				case 'n' : const_cast<string &>(m_nextServiceItemsFileName) = argv[++iter];
							break;
				case 'o' : const_cast<DistanceInKms &>(m_current_odometer) = atol(argv[++iter]);
							break;
				case 'p' : const_cast<DistanceInKms &>(m_prediction_kms) = atol(argv[++iter]);
							break;
				case 'P' : const_cast<date &>(m_purchase_date) = from_string(argv[++iter]);
							break;
				default : 
							{
								string err("Unrecognised option in input: -");
								err += argv[iter][1];
								throw logic_error(err);
							}
			}
		} else
		{
			string err("Unrecognised option in input");
			throw logic_error(err);
		}
	}

#ifdef __DEBUG
	cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ << " args parsed:" << endl;
    cout << "service guide file name: " << m_serviceGuideFileName << endl;
	cout << "service history file name: " << m_serviceHistoryFileName << endl;
	cout << "next service items file name: " << m_nextServiceItemsFileName << endl;
	cout << "current odometer reading: " << m_current_odometer << endl;
	cout << "prediction kms: " << m_prediction_kms << endl;
	cout << "purchase date: " << to_simple_string(m_purchase_date) << endl;
	cout << "===================================" << endl;
#endif

}

//================================================================================================

template <class PartDetailInTime, class Time, Time string_to_type(const string &)>
PartDetailInTime getPartInfo ( const string & i_rawString )
{
	deque<string> items = StringTokeniser::parse( i_rawString , ',' );

	return make_tuple( items[0] , items[1] , atol(items[2].c_str()) , string_to_type(items[3]) );
}

//================================================================================================

template <class PartsInfoInTime, class PartDetailInTime, class Time, Time string_to_type(const string &)>
void build_data_structure( const string & i_raw_string , PartsInfoInTime & io_partsGuideDetails )
{
	try
	{
		PartDetailInTime tempPartDetail = getPartInfo<PartDetailInTime, Time, string_to_type>( i_raw_string );

		Part_Action part_action;
		part_action = get<0>(tempPartDetail) + "_" + get<1>(tempPartDetail);

		io_partsGuideDetails[part_action] = make_tuple( get<2>(tempPartDetail) , get<3>(tempPartDetail) );

	} catch (logic_error &)
	{
		// do nothing and continue with the data structure already passed
	}

	return;
}

//================================================================================================

date string_to_date(const string & i_string_in_YYYY_MM_DD)
{
	return from_string(i_string_in_YYYY_MM_DD);

}

//================================================================================================

Days stringToDays(const string & i_days)
{
	return atol(i_days.c_str());
}

//================================================================================================

template <class PartsInfoInTime, class PartDetailInTime, class Time, Time string_to_type(const string &)>
PartsInfoInTime readServiceFile( const string & i_serviceFileName )
{
	try
	{
		FileStream serviceFile(i_serviceFileName.c_str(), ifstream::in);

#ifdef __DEBUG
	cout << "===================================" << endl;
	cout << "From function: " << __FUNCTION__ << " in location: after open file: " << i_serviceFileName << endl;
	cout << "===================================" << endl;
#endif
	
		PartsInfoInTime partsGuideDetails;

#ifdef __DEBUG
	cout << "===================================" << endl;
	cout << "From function: " << __FUNCTION__ << 
			" in location: before while loop of reading file: " << i_serviceFileName << endl;
	cout << "===================================" << endl;
#endif

		while ( ! serviceFile.eof() )
		{

#ifdef __DEBUG
	cout << "===================================" << endl;
	cout << "From function: " << __FUNCTION__ << 
			" in location: inside while loop of opening file: " << i_serviceFileName << endl;
	cout << "===================================" << endl;
#endif

			string tempLine;

#ifdef __DEBUG
	cout << "===================================" << endl;
	cout << "From function: " << __FUNCTION__ << 
			" in location: before reading line from file: " << i_serviceFileName << endl;
	cout << "===================================" << endl;
#endif
			getline ( serviceFile , tempLine );

#ifdef __DEBUG
	cout << "===================================" << endl;
	cout << "From function: " << __FUNCTION__ << 
			" in location: after reading line from file: " << i_serviceFileName << endl;
	cout << "===================================" << endl;
#endif

			build_data_structure<PartsInfoInTime , PartDetailInTime, 
								Time, string_to_type>( tempLine , partsGuideDetails );

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " in location: before peeking from file: " << i_serviceFileName << endl;
    cout << "===================================" << endl;
#endif

			serviceFile.peek();

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " in location: after peeking from file: " << i_serviceFileName << endl;
    cout << "===================================" << endl;
#endif
		}

		return partsGuideDetails;
	}
	catch(exception &e)
	{
		stringstream message;
		message << "Caught exception while trying to handle file: " << i_serviceFileName << endl
			<< "Exception messsage is: " << e.what() << endl;
		throw runtime_error(message.str());
	}
}

//================================================================================================

tuple<DistanceInKms, Days> convert_serviced_time_to_interval( DistanceInKms i_currentOdometer 
									, DistanceInKms i_predictionKms 
									, const tuple<DistanceInKms, date> & i_specific_part_serviced_time )
{
	DistanceInKms distance_to_predict = i_currentOdometer + i_predictionKms 
										- get<0>(i_specific_part_serviced_time);

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " current_odometer: " << i_currentOdometer <<
            " predictionKms: " << i_predictionKms <<
            " last serviced kms: " << get<0>(i_specific_part_serviced_time) << endl <<
            " distance to predict: " << distance_to_predict << endl;
    cout << "===================================" << endl;
#endif

	date current_time = day_clock::local_day();

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " current_time: " << to_simple_string(current_time)  << endl;
    cout << "===================================" << endl;
#endif

	date_duration date_diff = current_time - get<1>(i_specific_part_serviced_time);

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " last serviced time: " << get<1>(i_specific_part_serviced_time) <<
            " date_diff: " << date_diff.days() << endl;
    cout << "===================================" << endl;
#endif

	return make_tuple( distance_to_predict , date_diff.days() );
}

//================================================================================================

tuple<DistanceInKms, Days> fetch_serviced_detail_for_part_action( 
													const PartsInfoInDate & i_partsServiceHistory
													, const Part_Action & i_part_action
													, const DistanceInKms i_currentOdometer
													, const DistanceInKms i_predictionKms
													, const date & i_purchase_date )
{
	PartsInfoInDate::const_iterator service_hist_for_specific_part_ptr = 
												i_partsServiceHistory.find(i_part_action);

	if ( service_hist_for_specific_part_ptr == i_partsServiceHistory.end() )
	{

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " inside not finding part: " << i_part_action << " from service history" << endl;
    cout << "===================================" << endl;
#endif

		const_cast<PartsInfoInDate &>(i_partsServiceHistory)[i_part_action] = make_tuple( 0 , i_purchase_date );

		service_hist_for_specific_part_ptr = i_partsServiceHistory.find(i_part_action);
	}

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " after finding part: " << i_part_action << " from service history" << endl;
    cout << "===================================" << endl;
#endif

	const tuple<DistanceInKms, date> & specific_part_serviced_time = 
													service_hist_for_specific_part_ptr->second;

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " found distance: " << get<0>(specific_part_serviced_time) <<
            " and date: " << to_simple_string(get<1>(specific_part_serviced_time)) <<
            " from service history" << endl;
    cout << "===================================" << endl;
#endif

	return convert_serviced_time_to_interval( i_currentOdometer 
											, i_predictionKms 
											, specific_part_serviced_time );

}

//================================================================================================

forward_list<Part_Action> predictServiceItems( const PartsInfoInDays & i_partsServiceGuide 
									, const PartsInfoInDate & i_partsServiceHistory
									, const DistanceInKms i_currentOdometer
									, const DistanceInKms i_predictionKms
									, const date i_purchase_date )
{
	forward_list<Part_Action> o_parts_to_be_serviced;
	
	for ( PartsInfoInDays::const_iterator service_guide_for_specific_part_ptr = i_partsServiceGuide.begin() ;
			service_guide_for_specific_part_ptr != i_partsServiceGuide.end() ;
			service_guide_for_specific_part_ptr++ )
	{
		const Part_Action & part_action = service_guide_for_specific_part_ptr->first;

//#define __DEBUG

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " looking at part: " << part_action << " from service guide" << endl;
    cout << "===================================" << endl;
#endif

		const tuple<DistanceInKms, Days> & specific_part_action_guide_interval = 
												service_guide_for_specific_part_ptr->second;

		const tuple<DistanceInKms, Days> & specific_part_action_history_interval =
								fetch_serviced_detail_for_part_action( i_partsServiceHistory 
																		, part_action 
																		, i_currentOdometer
																		, i_predictionKms
																		, i_purchase_date );

		const DistanceInKms & kms_from_guide = get<0>(specific_part_action_guide_interval);
		const DistanceInKms & kms_from_hist = get<0>(specific_part_action_history_interval);
		const Days & days_from_guide = get<1>(specific_part_action_guide_interval);
		const Days & days_from_hist = get<1>(specific_part_action_history_interval);

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " analysing distance: " << kms_from_guide << " and days: " << days_from_guide
            << " from service guide" << endl;
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " analysing distance: " << kms_from_hist << " and days: " << days_from_hist
            << " from service history" << endl;
    cout << "===================================" << endl;
#endif

		if ( kms_from_hist >= kms_from_guide || days_from_hist >= days_from_guide )
		{
			o_parts_to_be_serviced.push_front(part_action);

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ <<
            " found part to service: " << part_action << endl;
    cout << "===================================" << endl;
#endif

//#undef __DEBUG
		}
	}

	return o_parts_to_be_serviced;
}

//================================================================================================

void writeServiceItems( const forward_list<Part_Action> & i_nextServiceItems, 
						const string & i_nextServiceItemsFileName )
{
	try
	{
		FileStream nextServiceFile(i_nextServiceItemsFileName.c_str(), ofstream::out);

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ << 
			" in location: after open file: " << i_nextServiceItemsFileName << endl;
    cout << "===================================" << endl;
#endif

		for ( auto next_service_item_ptr = i_nextServiceItems.begin() ;
				next_service_item_ptr != i_nextServiceItems.end() ;
				next_service_item_ptr++ )
		{
			nextServiceFile << *(next_service_item_ptr);
			nextServiceFile << endl;
		}

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ << 
			" in location: after writing all items to file: " << i_nextServiceItemsFileName << endl;
    cout << "===================================" << endl;
#endif

	}
    catch(exception &e)
    {
        stringstream message;
        message << "Caught exception while trying to handle file: " << i_nextServiceItemsFileName << endl
            << "Exception messsage is: " << e.what() << endl;
        throw runtime_error(message.str());
    }

}

//================================================================================================

int main( int argc , char *argv[] )
{
	bool status = true;

	try
	{
		CLIParser inputReader( argc , argv );

		PartsInfoInDays partsServiceGuide = readServiceFile<PartsInfoInDays, PartDetailInDays, Days, stringToDays>( inputReader.getServiceGuideFileName() );

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ << 
			" in location: after reading file: " << inputReader.getServiceGuideFileName() << endl;
    cout << "===================================" << endl;
#endif

		PartsInfoInDate partsServiceHistory = readServiceFile<PartsInfoInDate,PartDetailInDate, date, string_to_date>( inputReader.getServiceHistoryFileName() );

#ifdef __DEBUG
    cout << "===================================" << endl;
    cout << "From function: " << __FUNCTION__ << 
			" in location: after reading file: " << inputReader.getServiceHistoryFileName() << endl;
    cout << "===================================" << endl;
#endif

		forward_list<Part_Action> nextServiceItems = predictServiceItems( partsServiceGuide 
															, partsServiceHistory
															, inputReader.getCurrentOdometer()
															, inputReader.getPredictionKms()
															, inputReader.get_purchase_date() );

		writeServiceItems( nextServiceItems, inputReader.getNextServiceItemsFileName() );

	}
	catch (exception &e)
	{
		cerr<< "Caught exception:\n" "message : " << e.what() << endl;
		status = false;
	}

	return status;
}

//================================================================================================
