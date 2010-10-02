#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>


#define USERNAME 	""					// Enterprise Username
#define PASSWORD 	""				// Enterprise Password
#define WAITTIME 	5							// Interval to wait between checks (in seconds)
#define EMAIL    	""	// Email for notifications



int login();
int isClassFull(char *CRN);
char * getClassDetails(char *CRN);


int main(int argc, char **argv) {

	
	
	//printf(getClassDetails(argv[1]));
	
	
	//////////////////////////////////////////////
	// Step 1: Connect to Enterprise
	//////////////////////////////////////////////
	login();
	
	
	//////////////////////////////////////////////
	// Step 2: Get CRN's to check from the ruby database
	//////////////////////////////////////////////
	
	while (1) {
		
		// sqlite3 database handles...
		int retval;
		sqlite3_stmt *stmt;
		sqlite3 *handle;
		int q_size = 150;
		char *query = malloc(sizeof(char) * q_size);
		
		// Connect to the database
		retval = sqlite3_open("/Users/Dan/Personal/Code/Ruby/waitlist/db/development.sqlite3",&handle);
		if(retval)
			printf("Database connection failed\n");
		
		// grab the CRN's
		query = "SELECT * from waitlists";
		retval = sqlite3_prepare_v2(handle,query,-1,&stmt,0);
		if(retval)
			printf("Selecting data from DB Failed\n");
		
		int cols = sqlite3_column_count(stmt);
		
		while(1) {
			
			// On each new row, loop through the columns
			retval = sqlite3_step(stmt);
			if(retval == SQLITE_ROW) {
				int classOpen = 0;
				int * curCRN = NULL;
				char *email = NULL;
				
				for(int col=0 ; col<cols;col++) {
					const char *val = (const char*)sqlite3_column_text(stmt,col);
					
					//////////////////////////////////////////////
					// Step 3: Check if the course is open
					//////////////////////////////////////////////
					
					// Grab the course CRN
					if (strcmp(sqlite3_column_name(stmt,col), "crn") == 0) {
						printf(" Checking for CRN: %s\n", val);
						classOpen = !isClassFull(val);
						curCRN = val;
					}
					
					// Save the email
					if (strcmp(sqlite3_column_name(stmt,col), "email") == 0) {
						email = val;
					}
				}
				
				//////////////////////////////////////////////
				// Step 4: Notify user of an open course
				//////////////////////////////////////////////
				
				if (classOpen) {
					sendNotification(curCRN, email);
					printf("Course Open! Removing: %s\n", curCRN);
					// Remove user from the waitlist
					char * delQuery = malloc(sizeof(char) * 100);
					strcpy (delQuery, "DELETE FROM waitlists WHERE crn='");
					char * current = delQuery;
					current += strlen(delQuery);
					strcpy(current, curCRN);
					current += strlen(curCRN);
					strcpy(current, "' AND email='");
					current += strlen(current);
					strcpy(current, email);
					current += strlen(current);
					strcpy(current, "'");
					
					
					sqlite3_exec(handle,delQuery,0,0,0);
					
				}
				
			} else if(retval == SQLITE_DONE) {
				// All rows fetched
				break;
			} else {
				printf("Some error...\n");
			}
		}
		
		
		sqlite3_close(handle);
		
		
		sleep(WAITTIME); 
		
	}
	
}



//TODO: grab course title

/////////////////////////////////////////////////////////////////////////////
// Send notification of open course
/////////////////////////////////////////////////////////////////////////////
void sendNotification(char *CRN, char *emailAddress) {
	
	// Send notification email to client
	char command[500];
	char *current = command;
	strcpy(current, "echo \"Your course is now available! \" | mailx -s \"Waitlist course available\" ");
	current += 77;
	strcpy(current, emailAddress);
	system(command);
	
}



/////////////////////////////////////////////////////////////////////////////
// Handles all aspects of setting up a secure connection with enterprise
// Servers and logging in.
/////////////////////////////////////////////////////////////////////////////
int login(){
	
	// Log in to enterprise
	// - This page will create a cookie with a session ID that we can pass to the coming pages
	//   to validate with. Storing the cooking in cookies.txt
	char command[500];
	char *current = command;
	strcpy(current, "curl \"https://eas.admin.uillinois.edu/eas/servlet/EasLogin?redirect=https://webprod.admin.uillinois.edu/ssa/servlet/SelfServiceLogin?appName=edu.uillinois.aits.SelfServiceLogin&dad=BANPROD1\" -d \"principal=");
	current += 205;
	strcpy(current, USERNAME);
	current += strlen(USERNAME);
	strcpy(current, "&password=");
	current += strlen("&password=");
	strcpy(current, PASSWORD);
	current += strlen(PASSWORD);
	strcpy(current, "&login=Login&redirect=https%3A%2F%2Fwebprod.admin.uillinois.edu%2Fssa%2Fservlet%2FSelfServiceLogin%3FappName%3Dedu.uillinois.aits.SelfServiceLogin%26dad%3DBANPROD1\" -c cookies.txt -s");
	system(command);
	
	// Complete authentication
	// - We get directed back to this site, where more details are added to the cookie.
	system("curl \"https://webprod.admin.uillinois.edu/ssa/servlet/SelfServiceLogin?appName=edu.uillinois.aits.SelfServiceLogin&dad=BANPROD1\" -b cookies.txt -c cookies.txt &> details.txt");
	
	// Parse out the location details
	// - Here we get redirected yet again, but we need to parse out where to in the header returned by the prior command.
	char line[500];
	FILE* fp = fopen("details.txt","r");
	while( fgets(line,sizeof(line),fp) ) {
		if (strncmp(line, "< Location: ", 12) == 0) {
			break;
		}
	}
	fclose(fp);
	
	// This will complete the final stage of authentication by redirecting the last authentication page
	current = command;
	strcpy(current, "curl \"");
	current += 6;
	strcpy(current, line + 12);
	current += strlen(line + 12) - 2;
	strcpy(current, "\" -b cookies.txt --sslv3 -c cookies.txt -s");
	system(command);
	
	return 1;
	
	
}


/////////////////////////////////////////////////////////////////////////////
// Returns a boolean true if the given class CRN is still full
/////////////////////////////////////////////////////////////////////////////
int isClassFull(char *CRN){
	
	// Put in a request for the page we want to parse
	char command[500];
	char *current = command;
	strcpy(current, "curl \"https://ui2web1.apps.uillinois.edu/BANPROD1/bwckschd.p_disp_detail_sched?term_in=120108&crn_in=");
	current += 101;
	strcpy(current, CRN);
	current += strlen(CRN);
	strcpy(current, "\" -b cookies.txt --sslv3 -c cookies.txt -o out.htm -s");
	system(command);
	
	// Parse the output to see if the class is open
	char line[500];
	int found = 0;
	int seatsAvailable, crossCap, crossAct, i;
	char courseName[500];
	FILE *fp = fopen("out.htm","r");
	while( fgets(line,sizeof(line),fp) ) {
		if (strncmp(line, "<TABLE  CLASS=\"datadisplaytable\" SUMMARY=\"This layout table is used to present the seating numbers.\"", 99) == 0) {
			
			// Skip 10 lines
			for (i=0; i<10; i++)
				fgets(line,sizeof(line),fp);
			
			char *edit = strrchr(line, '<');
			*edit = '\0';
			seatsAvailable = atoi(line + 22);
			
			// Skip 10 lines
			for (i=0; i<10; i++)
				fgets(line,sizeof(line),fp);
			
			edit = strrchr(line, '<');
			*edit = '\0';
			crossCap = atoi(line + 22);
			
			// Next line
			fgets(line,sizeof(line),fp);
			
			edit = strrchr(line, '<');
			*edit = '\0';
			crossAct = atoi(line + 22);
			
			found = 1;
			
			break;
			
		} else if (strncmp(line, "<TH CLASS=\"ddlabel\" scope=\"row\" >", 33) == 0) {
			
			char *edit = strrchr(line + 33, '<');
			*edit = '\0';
			strcpy(courseName, line + 33);
			courseName[strlen(line+33) - 12] = '\0';
			
		}
	}
	
	fclose(fp);
	
	// Determine if class is full
	int classFull = 0;
	if (crossCap || crossAct) {
		if (crossCap <= crossAct)
			classFull = 1;
	} else {
		if (seatsAvailable < 1)
			classFull = 1;
	}
	
	return classFull;
	
}
/////////////////////////////////////////////////////////////////////////////
// Returns the course name and number associated with a given CRN
/////////////////////////////////////////////////////////////////////////////
char * getClassDetails(char *CRN){
	
	// Put in a request for the page we want to parse
	char command[500];
	char *current = command;
	strcpy(current, "curl \"https://ui2web1.apps.uillinois.edu/BANPROD1/bwckschd.p_disp_detail_sched?term_in=120108&crn_in=");
	current += 101;
	strcpy(current, CRN);
	current += strlen(CRN);
	strcpy(current, "\" -b cookies.txt --sslv3 -c cookies.txt -o out.htm -s");
	system(command);
	
	// Parse the output to see if the class is open
	char line[500];
	int found = 0;
	int seatsAvailable, crossCap, crossAct, i;
	char * courseName = malloc(500 * sizeof(char));
	FILE *fp = fopen("out.htm","r");
	while( fgets(line,sizeof(line),fp) ) {
		if (strncmp(line, "<TH CLASS=\"ddlabel\" scope=\"row\" >", 33) == 0) {
			
			char *edit = strrchr(line + 33, '<');
			*edit = '\0';
			strcpy(courseName, line + 33);
			courseName[strlen(line+33) - 12] = '\0';
			break;
		}
	}
	
	fclose(fp);
	return courseName;  
	
}



/////////////////////////////////////////////////////////////////////////////
// An attempt to auto register, still has far too many bugs to be usable...
/////////////////////////////////////////////////////////////////////////////
int registerForClass(int CRN) {
	/*
	 //system("curl \"https://ui2web1.apps.uillinois.edu/BANPROD1/twbkwbis.P_GenMenu?name=bmenu.P_RegAgreementLook\" -b cookies.txt --sslv3 -c cookies.txt -o test1.htm -v");
	 
	 
	 if (!classFull && 0) {
	 // Attempt to poll the page...
	 
	 //current = command;
	 //strcpy(current, "curl \"https://ui2web1.apps.uillinois.edu/BANPROD1/bwckschd.p_disp_detail_sched?term_in=120108&crn_in=");
	 //current += 101;
	 //strcpy(current, buf);
	 //current += strlen(buf);
	 //strcpy(current, "\" -b cookies.txt --sslv3 -c cookies.txt -o out2.htm -s");
	 //system(command);
	 
	 printf("Waiting...\n");
	 sleep(3);
	 
	 
	 system("curl \"https://ui2web1.apps.uillinois.edu/BANPROD1/twbkwbis.P_GenMenu?name=bmenu.P_RegAgreementLook\" -b cookies.txt --sslv3 -c cookies.txt -o test1.htm -s");
	 
	 system("curl \"https://ui2web1.apps.uillinois.edu/BANPROD1/bwskfcls.p_sel_crse_search\" -b cookies.txt --sslv3 -c cookies.txt -o test2.htm -s");
	 
	 
	 current = command;
	 strcpy(current, "curl \"https://ui2web1.apps.uillinois.edu/BANPROD1/bwskfcls.P_GetCrse\" -d \"term_in=120108&sel_subj=dummy&sel_day=dummy&sel_schd=dummy&sel_insm=dummy&sel_camp=dummy&sel_levl=dummy&sel_sess=dummy&sel_instr=dummy&sel_ptrm=dummy&sel_attr=dummy&sel_subj=ENG&sel_crse=100&sel_title=&sel_from_cred=&sel_to_cred=&sel_ptrm=%25&sel_sess=%25&sel_attr=%25&begin_hh=0&begin_mi=0&begin_ap=a&end_hh=0&end_mi=0&end_ap=a\" -b cookies.txt --sslv3 -c cookies.txt -o test.htm -s");
	 //current += 166;
	 //strcpy(current, buf);
	 //current += strlen(buf);
	 //strcpy(current, "+120108&assoc_term_in=120108&ADD_BTN=Register\" -b cookies.txt --sslv3 -c cookies.txt -o out.htm -s");
	 system(command);
	 
	 
	 
	 
	 // Enroll in the course!
	 current = command;
	 strcpy(current, "curl \"https://ui2web1.apps.uillinois.edu/BANPROD1/bwskfreg.P_AltPin1\" -d \"TERM_IN=120108&sel_crn=dummy&assoc_term_in=dummy&ADD_BTN=dummy&assoc_term_in=120108&sel_crn=");
	 current += 166;
	 strcpy(current, buf);
	 current += strlen(buf);
	 strcpy(current, "+120108&assoc_term_in=120108&ADD_BTN=Register\" -b cookies.txt --sslv3 -c cookies.txt -o out.htm -s");
	 system(command);
	 
	 // Check if it worked
	 fp = fopen("out.htm","r");
	 while( fgets(line,sizeof(line),fp) ) {
	 if (strncmp(line, "<meta http-equiv=\"refresh\" content=\"0;url=uiauthent.ss_timeout?ret_code=\">", 74) == 0) {
	 printf("Need to reauth... \n - Sending Credentials\n");
	 
	 // Clean Up
	 system("rm out.htm    ");
	 system("rm details.txt");
	 system("rm cookies.txt");
	 
	 // Log in to enterprise
	 // - This page will create a cookie with a session ID that we can pass to the coming pages
	 //   to validate with. Storing the cooking in cookies.txt
	 system("curl \"https://eas.admin.uillinois.edu/eas/servlet/EasLogin?redirect=https://webprod.admin.uillinois.edu/ssa/servlet/SelfServiceLogin?appName=edu.uillinois.aits.SelfServiceLogin&dad=BANPROD1\" -d \"principal=lambert6&password=vY48%40tD52Pk&login=Login&redirect=https%3A%2F%2Fwebprod.admin.uillinois.edu%2Fssa%2Fservlet%2FSelfServiceLogin%3FappName%3Dedu.uillinois.aits.SelfServiceLogin%26dad%3DBANPROD1\" -c cookies.txt -s");
	 
	 printf(" - Storing cookie\n");
	 // Complete authentication
	 // - We get directed back to this site, where more details are added to the cookie.
	 system("curl \"https://webprod.admin.uillinois.edu/ssa/servlet/SelfServiceLogin?appName=edu.uillinois.aits.SelfServiceLogin&dad=BANPROD1\" -b cookies.txt -c cookies.txt &> details.txt");
	 
	 printf(" - Parsing secure redirect\n");
	 // Parse out the location details
	 // - Here we get redirected yet again, but we need to parse out where to in the header returned by the prior command.
	 char line2[500];
	 FILE* fp2 = fopen("details.txt","r");
	 while( fgets(line2,sizeof(line2),fp2) ) {
	 if (strncmp(line2, "< Location: ", 12) == 0) {
	 break;
	 }
	 }
	 fclose(fp);
	 
	 printf(" - Launching secure redirect\n");
	 // This will complete the final stage of authentication by redirecting the last authentication page
	 char command2[500];
	 char *current2 = command2;
	 strcpy(current2, "curl \"");
	 current2 += 6;
	 strcpy(current2, line2 + 12);
	 current2 += strlen(line2 + 12) - 2;
	 strcpy(current2, "\" -b cookies.txt --sslv3 -c cookies.txt -s");
	 system(command2);
	 
	 
	 printf(" - Adding course\n");
	 // Now rerun the original command
	 system(command);
	 
	 
	 
	 }
	 }
	 
	 
	 
	 }
	 
	 
	 
	 */
	
}



