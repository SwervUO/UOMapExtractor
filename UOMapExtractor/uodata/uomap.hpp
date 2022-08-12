//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef uomap_hpp
#define uomap_hpp

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <utility>
#include <tuple>
#include <filesystem>

#include "mapblock.hpp"
#include "uopfile.hpp"

/*
 Supports manipulation of a UO map.
 It supports the following functions:
 	Initialize a map (all terrain 0, no art)
 	Load terrain (mul or uop)
 	Load art (mul)
 	Apply diff (art or terrain)
 	Save art (mul)
 	Save terrain (mul or uop)
 	Retrieve the terrain tileid and altitude for an x,y
 	Modify the terrain tileid and altitude for an x,y
 	Retrieve all art (tileid, altitude, hue) for an x,y
 	Retrieve all art (tileid, altitude,hue) for an x,y,z
    Add art (tileid, altitude,hue) for an x,y
    Remove art for an x,y
    Remove art for an x,y,z
 */


//=================================================================================
class uomap_t : public uopfile {
	constexpr static auto totalmaps = 6 ;
	static constexpr std::array<std::pair<int,int>,totalmaps> mapsizes{{
		{7168,4096},{7168,4096},{2304,1600},
		{2560,2048},{1448,1448},{1280,4096}
	}};

	std::vector<terrainblock_t> terraindata ;
	std::vector<artblock_t> artdata ;
	
	int mapnumber ;
	int width ;
	int height ;
	
	auto calcBlock(int x, int y) const -> int ;
	auto calcXYForBlock(int block) const -> std::pair<int, int> ;
	auto calcBlockOffset(int x, int y) const -> std::tuple<int, int,int> ;
	

	//  UOP methods
	auto processEntry(std::size_t entry, std::size_t index, std::vector<std::uint8_t> &data) ->bool final ;

	auto entriesToWrite()const ->int final ;
	auto entryForWrite(int entry)->std::vector<unsigned char> final ;

	auto writeHash(int entry)->std::string ;

public:
	static auto maxmap() ->size_t {return mapsizes.size();}
	uomap_t(int mapnum=0, int width=0, int height = 0);
	auto setSize(int width, int height) ->void ;
	auto size() const ->std::pair<int,int> {return std::make_pair(width,height);}

	auto loadTerrainMul(const std::filesystem::path &path) ->bool ;
	auto loadTerrainUOP(const std::filesystem::path &path) ->bool ;
	auto applyTerrainDiff(const std::string &difflpath,const std::string &diffpath) ->bool ;
	auto writeTerrainMul(const std::string &path) const ->bool ;
	auto writeTerrainUOP(const std::string &path)  ->bool ;
	
	auto loadArt(const std::string &idxpath, const std::string &mulpath) ->bool ;
	auto applyArtDiff(const std::string &difflpath, const std::string &diffipath, const std::string &diffpath) ->bool ;
	auto writeArt(const std::string &idxpath, const std::string &mulpath)const  ->bool ;
	
	auto terrain(int x, int y) const ->std::pair<std::uint16_t,std::int8_t> ;
	auto terrain(int x, int y, std::uint16_t tileid, std::int8_t altitude) ->void ;
	auto art(int x, int y) const ->std::vector<std::tuple<std::uint16_t,std::int8_t,std::uint16_t>> ;
	auto art(int x, int y, std::uint16_t tileid, std::int8_t altitude, std::uint16_t hue)  ->void;
	auto art(int x, int y,int z) const ->std::vector<std::tuple<std::uint16_t,std::int8_t,std::uint16_t>> ;
	auto remove(int x, int y) ->void ;
	auto remove(int x, int y, int z) ->void ;


};

#endif /* uomap_hpp */
