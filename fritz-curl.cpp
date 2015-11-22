/***************************************************************************
 *          ______    _ _              _   _______ _ 
 *          |  ___|  (_) |            | | | | ___ \ |
 *          | |_ _ __ _| |_ ____   ___| | | | |_/ / |
 *          |  _| '__| | __|_  /  / __| | | |    /| | 
 *          | | | |  | | |_ / /  | (__| |_| | |\ \| |____
 *          \_| |_|  |_|\__/___|  \___|\___/\_| \_\_____/
 * 
 * 2015-11-02: copyleft(c) 2015 Allard Lamberink
 *
 * API for the AVM Fritz!BOX to switch WLAN on/off from the command line
 *
 *
 ***************************************************************************/
// ignore MD5 deprecated warnings on OSX
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <iostream>
#include <string>
#include <set>
#include <exception>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <curl/curl.h>

#include <openssl/md5.h>

using namespace std;
using boost::property_tree::ptree;

bool bDebug = false;

void readfile(const std::string &filepath,std::string &buffer){
        std::ifstream fin(filepath.c_str());
        getline(fin, buffer, char(-1)); 
        fin.close();
}


size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	string data((const char*) ptr, (size_t) size * nmemb);
	*((stringstream*) stream) << data << endl;
	return size * nmemb;
}


string getmd5sum(string inpstr)
{
	unsigned char digest[MD5_DIGEST_LENGTH];
	char inpa[inpstr.size()];
	strcpy(inpa, inpstr.c_str());

	MD5((unsigned char*)&inpa, strlen(inpa), (unsigned char*)&digest);
	char mdString[33];
	for(int i = 0; i < 16; i++)
		sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

	return string(mdString);
}


string executeCurlAction(string fburl, string fbport, string upnpaction, string urn, string functionname, string postfields)
{
	stringstream kout;

	CURL *curl;
	//get a curl handle
	curl = curl_easy_init();
	if(curl)
       	{
		CURLcode res;
		res = curl_global_init(CURL_GLOBAL_DEFAULT);
		
		/* Check for errors */
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_global_init() failed: %s\n",
		        curl_easy_strerror(res));
			return "-1";
		}

		curl_easy_setopt(curl, CURLOPT_URL, (fburl + ":" + fbport + upnpaction).c_str());

		struct curl_slist *c_chunk = NULL;

		string s_soapaction = "SoapAction: " + urn + "#" + functionname;

		c_chunk = curl_slist_append(c_chunk, "Content-Type: text/xml; charset=\"utf-8\"");
		c_chunk = curl_slist_append(c_chunk, s_soapaction.c_str());

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, c_chunk);

		/* Now specify we want to POST data */
        	curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postfields.size());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields.c_str());
		/* get verbose debug output please */
		if(bDebug) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &kout);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
	
		/* Check for errors */
		if(res != CURLE_OK)
      			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		if(c_chunk != NULL)
			curl_slist_free_all(c_chunk);

		curl_easy_cleanup(curl);
	}
	else
	{
		cout << "could not init curl" <<endl;
	}
	if(bDebug) cout << "result=" << kout.str() << endl;
	return kout.str();
}


string getXmlProp(string xmlstring, string xpath)
{
	string retval = "Error reading XML property";

	//// Read the XML config string into the property tree. Catch any exception
	try {
		ptree pt;
		stringstream xmlstream; xmlstream << xmlstring;
		read_xml(xmlstream, pt);
		retval = pt.get<std::string>(xpath);
	}
	catch (std::exception &e)
	{
		std::cout << "Error reading xml property: " << e.what() << "\n";
	}
	return retval;
}


int main(int argc, char* argv[]) { 
	if (argc < 2)
	{
		cout << "Fritz-cURL version 0.1" << endl;
		cout << "No arguments given, example usage:" << endl;
		cout << "wlan24 0 (switch wlan 2.4 GHz off" << endl;
		cout << "wlan24 1 (switch wlan 2.4 GHz on" << endl;
		cout << "wlan24 2 (return wlan 2.4 GHz status" << endl;
	}
	if (argc == 3)
	{
		string functionname, argname, upnpaction, urn = "";
		upnpaction = "/upnp/control/wlanconfig1"; // currently only wlan support
		urn = "urn:dslforum-org:service:WLANConfiguration:1"; //=serviceType and !ID
		switch(argv[2][0])
		{
			case '0':
			       	functionname = "SetEnable";
			 	argname = "<NewEnable>0</NewEnable>";
				break;
			case '1':
			       	functionname = "SetEnable";
			 	argname = "<NewEnable>1</NewEnable>";
				break;
			case '2':
			       	functionname = "GetInfo";
			 	argname = "";
				break;
		}

		const string runningpath = argv[0];
		const string settingsfullpath = runningpath.substr(0,runningpath.find_last_of("/")) + "/settings.xml";

		string settingsstr = "";
		/* read settings file */
		try
	        {
			readfile(settingsfullpath, settingsstr);
		}
		catch (std::exception &e)
		{
			perror("Error loading settings file, make sure settings.xml exists and is readable. Original errormessage: ");
		}

		/* initialize variables */ 
	        if(settingsstr.empty())
		{
			perror("Error loading settings file, make sure settings.xml exists, is readable and is not empty.");
			return 1;
		}

		string fbuser = getXmlProp(settingsstr, "settings.fbuser");
		string fbpwd = getXmlProp(settingsstr, "settings.fbpwd");
		string fburl = "http://" + getXmlProp(settingsstr, "settings.fburl");
		string fbport = getXmlProp(settingsstr, "settings.fbport");
		string debugstr = getXmlProp(settingsstr, "settings.debug");
		bDebug = debugstr == "True";

		if(bDebug) cout << "functionname=" << functionname << " and argname=" << argname << endl;

		// if the fritzbox is password protected, then first an initchallenge needs to be sent in order to get the Nonce and the Realm

		string initdata="<?xml version=\"1.0\" encoding=\"utf-8\"?> <s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Header><h:InitChallenge xmlns:h=\"http://soap-authentication.org/digest/2001/10/\" s:mustUnderstand=\"1\"><UserID>" + fbuser + "</UserID></h:InitChallenge></s:Header><s:Body><u:" + functionname + " xmlns:u=\"" + urn + "\"></u:" + functionname + "></s:Body></s:Envelope>";

		if (bDebug)
		{	
		    cout << "sending initdata=" + initdata + "\r\n\r\n" << endl;
		}

		/* send the initdata using curl */
		string out = executeCurlAction(fburl, fbport, upnpaction, urn, functionname, initdata);
		if (bDebug)
		{
			cout << "result = " + out << endl;
		}	

		/* read the nonce and realm from the curlaction result */
		string nonce = getXmlProp(out, "s:Envelope.s:Header.h:Challenge.Nonce");
		string realm = getXmlProp(out, "s:Envelope.s:Header.h:Challenge.Realm");
		if (bDebug)
		{
			fprintf(stdout, "\r\n\r\nAllard:nonce=%s en realm=%s \r\n\r\n", nonce.c_str(), realm.c_str());
		}

		string authstr=fbuser + ":" + realm + ":" + fbpwd;
		string md5authstr=getmd5sum(authstr);
		string auth = getmd5sum(md5authstr + ":" + nonce);
		
		if (bDebug)
		{
			cout << "sending part2" << endl;
		}

		string actiondata="<?xml version=\"1.0\" encoding=\"utf-8\"?> <s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Header><h:ClientAuth xmlns:h=\"http://soap-authentication.org/digest/2001/10/\" s:mustUnderstand=\"1\"><Nonce>" + nonce + "</Nonce><Auth>" + auth + "</Auth><UserID>" + fbuser + "</UserID><Realm>" + realm + "</Realm></h:ClientAuth></s:Header><s:Body><u:" + functionname + " xmlns:u=\"" + urn + "\">" + argname + "</u:" + functionname + "></s:Body></s:Envelope>";

		out = executeCurlAction(fburl, fbport, upnpaction, urn, functionname, actiondata);

		// TODO: read the new nonce
		// TODO: read out the new WLAN status to verify whether the command was executed successfully or not
		// example: cout << "callist url=" + getXmlProp(out, "s:Envelope.s:Body.u:GetCallListResponse.NewCallListURL") << endl;
		//
		// TODO: cleanup all used variables / objects!!! 
		// TODO: add support for security port
	}
	else
	{ //other than 3 arguments, do nothing
	}
	return 0;
}
