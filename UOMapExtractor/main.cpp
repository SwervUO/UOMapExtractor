//
//Copyright © 2022 Charles Kerr. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include "uomap.hpp"
#include "strutil.hpp"

using namespace std::string_literals;

int main(int argc, const char * argv[]) {
#if defined (_WIN32)
	auto basedir = std::filesystem::path("C:\\Program Files (x86)\\Electronic Arts\\Ultima Online Classic");
#else
	auto basedir = std::filesystem::path("/Users/charleskerr/Documents/uoclient");
#endif
	if (argc >1) {
		basedir = std::filesystem::path(argv[1]);
	}
	
	for (auto mapnum = 0 ; mapnum < 6 ; ++mapnum){
		auto width = 0 ;
		auto height = 0 ;
		auto sourcemap = basedir / std::filesystem::path(strutil::format("map%iLegacyMUL.uop",mapnum));
		auto artidx = basedir / std::filesystem::path(strutil::format("staidx%i.mul",mapnum));
		auto artmul = basedir / std::filesystem::path(strutil::format("statics%i.mul",mapnum));
		auto difl =basedir / std::filesystem::path(strutil::format("stadifl%i.mul",mapnum));
		auto difi =basedir / std::filesystem::path(strutil::format("stadifi%i.mul",mapnum));
		auto dif =basedir / std::filesystem::path(strutil::format("stadif%i.mul",mapnum));
		
		auto commandlist = strutil::format("buildmap%i.lst",mapnum);
		
		
		auto uomap = uomap_t(mapnum,width,height) ;
		auto [twidth,theight] = uomap.size() ;
		width = twidth ;
		height = theight ;
		if (uomap.loadTerrainUOP(sourcemap.string())) {
			if (uomap.loadArt(artidx.string(), artmul.string())){
				
				std::cout <<"Generating map " << mapnum << std::endl;
				auto output = std::ofstream(commandlist) ;
				if (!output.is_open()){
					std::cerr << "Unable to create: "<<commandlist<<std::endl;
					break ;
				}
				output << "//Generation of map " << mapnum << std::endl;
				output << "//Terrain from: "<<sourcemap.string() << std::endl;
				output <<"//" << std::endl;
				output << "//Art from: "<<artidx.string() << std::endl;
				output << "//Art from: "<<artmul.string() << std::endl;
				output <<"//" << std::endl;
				output <<"//Art diff from: " << difl.string() << std::endl;
				output <<"//Art diff from: " << difi.string() << std::endl;
				output <<"//Art diff from: " << dif.string() << std::endl;
				
				
				if (!uomap.applyArtDiff(difl.string(), difi.string(), dif.string())) {
					std::cerr <<"Unable to load art diffs, continuing without"<<std::endl;
				}
				output <<"//" << std::endl;
				output <<"init "<<mapnum<<","<<width<<","<<height << std::endl;
				
				output <<"msg Populating map" << std::endl;
				for (auto y = 0 ; y<height ;++y){
					if (y%8 ==0) {
						//std::cout <<y <<" of "<<height<<std::endl;
						output <<"//" << std::endl;
						output<<"// Starting section y="<<y<<std::endl;
						output <<"msg Starting section y = " <<y<<std::endl;
						output <<"//" << std::endl;
					}
					for (auto x = 0 ; x<width;++x) {
						auto [terid,teralt] = uomap.terrain(x, y);
						output<<"add terrain,"<<x<<","<<y<<","<<strutil::ntos(terid,strutil::radix_t::hex,true,4)<<","<<static_cast<int>(teralt)<<std::endl;
						auto cells = uomap.art(x,y) ;
						for (auto cell: cells){
							auto tileid = strutil::ntos(std::get<0>(cell),strutil::radix_t::hex,true,4) ;
							auto alt = static_cast<int>(std::get<1>(cell));
							auto hue = std::get<2>(cell) ;
							output<<"add art,"<<x<<","<<y<<","<<tileid<<","<<alt<<","<<hue<<std::endl;
						}
					}
				}
			}
			else {
				std::cerr <<"Unable to load art, skipping" << std::endl;
			}
		}
		else {
			std::cerr <<"Unable to load terrain, skipping" << std::endl;
		}
	}
	return 0;
}
