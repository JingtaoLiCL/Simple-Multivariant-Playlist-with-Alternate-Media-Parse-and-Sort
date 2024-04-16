# Simple Multivariant Playlist with Alternate Media Parse and Sort

## Overview
This C++ application demonstrates fetching, parsing, sorting, and writing an HLS multivariant playlist. It is designed to handle playlists with alternate media options, sorting the streams by bandwidth and writing the sorted data back into a new .m3u8 file.

## Features
- **Fetch Playlist**: Downloads an HLS playlist from a specified URL using the libcurl library.
- **Parse Playlist**: Extracts individual stream details and URI from the playlist content.
- **Sort Streams**: Organizes streams by their bandwidth in non-decreasing order.
- **Write to File**: Outputs the sorted stream information with any non-stream related data back into an .m3u8 formatted file.

## Prerequisites
Before building and running this project, ensure you have the following installed:
- C++ Compiler
- [libcurl](https://curl.se/libcurl/) for handling HTTP requests to fetch

## Running using MacOS

### Installing libcurl
On MacOS:
```
brew install curl
```

### Compile
```
g++ -std=c++17 -o playlist_parser playlist_parser.cpp -lcurl
```

### Running the Application
```
./playlist_parser
```

This will fetch the playlist, sort it by bandwidth, and output the sorted playlist into a file named sorted_playlist.m3u8.

## Configuration
Modify the application directly in the playlist_parser.cpp file to change settings like the playlist URL or output filename:
```
std::string url = "/path/to/playlist.m3u8";  // Replace with your playlist URL
```
## Output
The output will be written to sorted_playlist.m3u8, which will be formatted as follows:

Non-stream-related lines at the top
Stream details sorted by bandwidth, each followed by its respective URI

A "sample_sorted_playlist.m3u8" is also included in this repo.

## Thoughts and Further Improvements
###Time used in coding: one and half hour.
###Time spent writing Readme.md: one hour.
This parser only included support the tag "EXT-X-STREAM-INF" to identify Rendition Playlist (treating other tags as other information in general) and only sorts by bandwidth. For later revisions, more supported tags needs to be added as well as adding more functions to support manipulation of different tags and its parameters.
