#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <curl/curl.h>

// Struct to store information about each stream
struct StreamInfo {
    std::string details;  // Stores the #EXT-X-STREAM-INF line
    int bandwidth;        // Bandwidth extracted from the details
    std::string uri;      // URI following the details

    StreamInfo(const std::string& details, int bandwidth, const std::string& uri)
        : details(details), bandwidth(bandwidth), uri(uri) {}
};

// Comparator function to sort streams by bandwidth
bool compareByBandwidth(const StreamInfo& a, const StreamInfo& b) {
    return a.bandwidth < b.bandwidth;
}

// Callback function for CURL; used to append data to a string
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc &e) {
        return 0;  // Return 0 on allocation error to stop curl
    }
    return newLength;
}

// Function to fetch the playlist content from a given URL
std::string fetchPlaylist(const std::string& url) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    return readBuffer;
}

// Parses the playlist content and extracts stream and non-stream information
std::pair<std::vector<StreamInfo>, std::vector<std::string>> parseMultivariantPlaylistContent(const std::string& content) {
    std::vector<StreamInfo> streams;
    std::vector<std::string> nonStreamLines;
    std::size_t pos = 0;
    std::string token;
    std::string delimiter = "\n";
    bool nextLineIsUri = false;
    std::string currentDetails;
    int currentBandwidth;

    std::string contentCopy = content;  // Make a copy to manipulate

    while ((pos = contentCopy.find(delimiter)) != std::string::npos) {
        token = contentCopy.substr(0, pos);
        contentCopy.erase(0, pos + delimiter.length());

        if (nextLineIsUri) {
            streams.push_back(StreamInfo(currentDetails, currentBandwidth, token));
            nextLineIsUri = false;
            continue;
        }

        if (token.find("#EXT-X-STREAM-INF:") != std::string::npos) {
            std::size_t bandwidthPos = token.find("BANDWIDTH=");
            if (bandwidthPos != std::string::npos) {
                std::string bandwidthStr = token.substr(bandwidthPos + 10, token.find(",", bandwidthPos) - bandwidthPos - 10);
                try {
                    currentBandwidth = std::stoi(bandwidthStr);
                    currentDetails = token;
                    nextLineIsUri = true; // Prepare to capture the next line as URI
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing bandwidth: " << e.what() << std::endl;
                }
            }
        } else if (!token.empty()) {
            nonStreamLines.push_back(token);  // Collect non-stream lines
        }
    }
    return {streams, nonStreamLines};
}

// Writes the sorted playlist to a file
void writePlaylistToFile(const std::vector<StreamInfo>& streams, const std::vector<std::string>& nonStreamLines, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    // Write non-stream lines first (e.g., headers, metadata)
    for (const auto& line : nonStreamLines) {
        outFile << line << std::endl;
    }

    // Write stream information including URI
    for (const auto& stream : streams) {
        outFile << stream.details << std::endl;
        outFile << stream.uri << std::endl;
    }

    outFile.close();
    std::cout << "Playlist written to " << filename << std::endl;
}

// Main function to execute fetching, parsing, sorting, and writing playlist
int main() {
    std::string url = "https://lw.bamgrid.com/2.0/hls/vod/bam/ms02/hls/dplus/bao/master_unenc_hdr10_all.m3u8";
    std::string content = fetchPlaylist(url);

    if (content.empty()) {
        std::cerr << "Failed to fetch the playlist." << std::endl;
        return 1;
    }

    auto [streams, nonStreamLines] = parseMultivariantPlaylistContent(content);
    std::sort(streams.begin(), streams.end(), compareByBandwidth);

    writePlaylistToFile(streams, nonStreamLines, "sorted_playlist.m3u8");

    return 0;
}
