#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <pthread.h>

using namespace std;

#define NUM_THREADS 3

struct DownloadFileData {
    const char* srcURL;
    const char* destFile;
};

// callback function writes data to a std::ostream
static size_t data_write(void* buf, size_t size, size_t nmemb, void* userp)
{
	if (userp)
	{
		std::ostream& os = *static_cast<std::ostream*>(userp);
		std::streamsize len = size * nmemb;
		if (os.write(static_cast<char*>(buf), len))
			return len;
	}

	return 0;
}


CURLcode curl_read(const std::string& url, std::ostream& os, long timeout = 30)
{
	CURLcode code(CURLE_FAILED_INIT);
	CURL* curl = curl_easy_init();

	if (curl)
	{
		if (CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &data_write))
			&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FILE, &os))
			&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str())))
		{
			code = curl_easy_perform(curl);
            stringstream ss;
            ss << code;
            printf(ss.str().c_str());
            
		}
		curl_easy_cleanup(curl);
	}
	return code;
}

void *DownloadImage(void *threadarg) {
    struct DownloadFileData *threadData;
    threadData = (DownloadFileData*)threadarg;

    std::ofstream ofs(threadData->destFile, std::ostream::binary);

	if (CURLE_OK == curl_read(threadData->srcURL, ofs))
	{
		printf("Saved");
        return 0;
	} else {
        printf("Failed");
    }

    pthread_exit(NULL);
    
}


// Image URL
std::string url = "https://images.unsplash.com/photo-1540039452455-b76ea34cf66a";

int main()
{
	curl_global_init(CURL_GLOBAL_ALL);

    pthread_t thread[NUM_THREADS];
    
    int num = NUM_THREADS;

    int execute;

    for (int i = 0; i < NUM_THREADS; i++) {
        struct DownloadFileData fData[NUM_THREADS];
        stringstream ss;
        ss << i;
        string destFileName = "downloadedFile";
        destFileName.append(ss.str());
        destFileName.append(".jpg");
        fData[i].srcURL = url.c_str();
        fData[i].destFile = destFileName.c_str();
        cout << "Creating thread " << i << endl;
        execute = pthread_create(&thread[i], NULL, DownloadImage, (void*)&fData[i]);

        if (execute) {
            cout << "Error:unable to create thread," << execute << endl;
            exit(-1);
        }
    }
    pthread_exit(NULL);
    return 0;

	curl_global_cleanup();
}