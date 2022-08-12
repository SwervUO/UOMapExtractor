//Copyright © 2022 Charles Kerr. All rights reserved.

#include "mapblock.hpp"
#include "strutil.hpp"

#include <iostream>
#include <algorithm>
#include <iterator>

using namespace std::string_literals;

//=================================================================================
//		Terrain type structures
//=================================================================================

//=================================================================================
terrainblock_t::terrainblock_t(std::int32_t blockheader ) {
	blockdata.resize(196,0);
	std::copy(reinterpret_cast<std::uint8_t*>(&blockheader),reinterpret_cast<std::uint8_t*>(&blockheader)+4,blockdata.data());
	
}

//=================================================================================
terrainblock_t::terrainblock_t(const std::uint8_t* data){
	blockdata.resize(196,0);
	if (data != nullptr){
		std::copy(data,data+196,blockdata.data());
	}

}

//=================================================================================
auto terrainblock_t::header() const ->std::int32_t {
	return  *reinterpret_cast<const std::int32_t*>(blockdata.data()) ;
}
//=================================================================================
auto terrainblock_t::header(std::int32_t value) ->void{
	*reinterpret_cast< std::int32_t*>(blockdata.data()) = value ;
}

//=================================================================================
auto terrainblock_t::raw() const -> const std::vector<std::uint8_t>& {
	return blockdata ;
}

//=================================================================================
auto terrainblock_t::raw()  ->  std::vector<std::uint8_t>& {
	return blockdata ;
}

//=================================================================================
auto terrainblock_t::terrain(int x, int y) const -> std::pair<std::uint16_t,std::int8_t> {
	auto offset = (x*3) + (y*24) + 4 ;
	auto tileid = std::uint16_t(0) ;
	auto altitude = std::int8_t(0) ;
	std::copy(blockdata.data()+offset,blockdata.data()+offset + 2,reinterpret_cast<std::uint8_t*>(&tileid));
	std::copy(blockdata.data()+offset+2,blockdata.data()+offset + 3,reinterpret_cast<std::uint8_t*>(&altitude));

	return std::make_pair(tileid, altitude);
}
//=================================================================================
auto terrainblock_t::terrain(int x, int y, std::uint16_t tileid, std::int8_t altitude) ->void {
	auto offset = (x*3) + (y*24) + 4 ;
	std::copy(reinterpret_cast<std::uint8_t*>(&tileid),reinterpret_cast<std::uint8_t*>(&tileid)+2,blockdata.data()+offset);
	std::copy(reinterpret_cast<std::uint8_t*>(&altitude),reinterpret_cast<std::uint8_t*>(&altitude)+1,blockdata.data()+offset+2);
}

//=================================================================================
auto terrainblock_t::fill(std::uint16_t tileid, std::int8_t altitude) ->void {
	for (auto x = 0;x<8;++x){
		for (auto y=0;y<8;++y){
			this->terrain(x, y, tileid, altitude);
		}
	}
}


//=================================================================================


//=================================================================================
//		Art type structures
//=================================================================================


//=================================================================================
artblock_t::artblock_t(const std::uint8_t *data, size_t size) :artblock_t(){
	if (data != nullptr) {
		if (size != 0) {
			blockdata.resize(size,0);
			std::copy(data,data+size,blockdata.data());
		}
	}
}
//=================================================================================
artblock_t::artblock_t() {
	blockdata.clear();
	blockdata.resize(0);
}

//=================================================================================
auto artblock_t::size() const ->size_t {
	return blockdata.size();
}

//=================================================================================
auto artblock_t::raw() const -> const std::vector<std::uint8_t>& {
	return blockdata;
}
//=================================================================================
auto artblock_t::raw()  -> std::vector<std::uint8_t>& {
	return blockdata;
}


//=================================================================================
auto artblock_t::clear() ->void {
	blockdata.clear();
	blockdata.resize(0) ;
}


//================================================================================
auto artblock_t::art(int x, int y, std::uint16_t tileid, std::int8_t altitude, std::uint16_t hue ) ->void {
	auto size = blockdata.size() ;
	blockdata.resize(size+7);
	auto xloc = static_cast<std::uint8_t>(x) ;
	auto yloc = static_cast<std::uint8_t>(y) ;
	std::copy(reinterpret_cast<std::uint8_t*>(&tileid),reinterpret_cast<std::uint8_t*>(&tileid)+2,blockdata.data()+size);
	std::copy(&xloc,&xloc + 1,blockdata.data()+size + 2);
	std::copy(&yloc,&yloc + 1,blockdata.data()+size + 3);
	std::copy(reinterpret_cast<std::uint8_t*>(&altitude),reinterpret_cast<std::uint8_t*>(&altitude) + 1,blockdata.data()+size + 4);
	std::copy(reinterpret_cast<std::uint8_t*>(&hue),reinterpret_cast<std::uint8_t*>(&hue)+2,blockdata.data()+size+5);
}

//================================================================================
auto artblock_t::art(int x, int y) const -> std::vector<std::tuple<std::uint16_t,std::int8_t,std::uint16_t>> {
	auto rvalue = std::vector<std::tuple<std::uint16_t,std::int8_t,std::uint16_t>>() ;
	auto tileid = std::uint16_t(0) ;
	auto altitude = std::int8_t(0) ;
	auto hue = std::uint16_t(0) ;
	auto xloc = static_cast<std::uint8_t>(x) ;
	auto yloc = static_cast<std::uint8_t>(y) ;
	int offset = 0 ;
	while (offset < blockdata.size()){
		std::copy(blockdata.data()+offset,blockdata.data() + offset + 2, reinterpret_cast<std::uint8_t*>(&tileid));
		std::copy(blockdata.data()+offset+2,blockdata.data() + offset + 3, &xloc);
		std::copy(blockdata.data()+offset+3,blockdata.data() + offset + 4, &yloc);
		std::copy(blockdata.data()+offset+4,blockdata.data() + offset + 5, reinterpret_cast<std::uint8_t*>(&altitude));
		std::copy(blockdata.data()+offset+5,blockdata.data() + offset + 7, reinterpret_cast<std::uint8_t*>(&hue));
		if ((static_cast<int>(xloc) == x) && (static_cast<int>(yloc) == y)){
			rvalue.push_back(std::make_tuple(tileid,altitude,hue));
		}
		offset += 7 ;
	}
	return rvalue ;

}
//================================================================================
auto artblock_t::art(int x, int y,int alt) const -> std::vector<std::tuple<std::uint16_t,std::int8_t,std::uint16_t>> {
	auto rvalue = std::vector<std::tuple<std::uint16_t,std::int8_t,std::uint16_t>>() ;
	auto tileid = std::uint16_t(0) ;
	auto altitude = std::int8_t(0) ;
	auto hue = std::uint16_t(0) ;
	auto xloc = static_cast<std::uint8_t>(x) ;
	auto yloc = static_cast<std::uint8_t>(y) ;
	int offset = 0 ;
	while (offset < blockdata.size()){
		std::copy(blockdata.data()+offset,blockdata.data() + offset + 2, reinterpret_cast<std::uint8_t*>(&tileid));
		std::copy(blockdata.data()+offset+2,blockdata.data() + offset + 3, &xloc);
		std::copy(blockdata.data()+offset+3,blockdata.data() + offset + 4, &yloc);
		std::copy(blockdata.data()+offset+4,blockdata.data() + offset + 5, reinterpret_cast<std::uint8_t*>(&altitude));
		std::copy(blockdata.data()+offset+5,blockdata.data() + offset + 7, reinterpret_cast<std::uint8_t*>(&hue));
		if ((static_cast<int>(xloc) == x) && (static_cast<int>(yloc) == y) && (static_cast<int>(altitude)==alt)){
			rvalue.push_back(std::make_tuple(tileid,altitude,hue));
		}
		offset += 7 ;
	}
	return rvalue ;

}
//================================================================================
auto artblock_t::remove(int x, int y) ->void {
	auto temp = std::vector<std::uint8_t>() ;
	auto xloc = std::uint8_t(0) ;
	auto yloc = std::uint8_t(0) ;
	auto offset = 0 ;
	while (offset < blockdata.size()){
		std::copy(blockdata.data()+offset+2,blockdata.data()+offset+3,&xloc) ;
		std::copy(blockdata.data()+offset+3,blockdata.data()+offset+4,&xloc) ;
		if ((static_cast<int>(xloc) != x) || (static_cast<int>(yloc) != y)){
			auto size = temp.size() ;
			temp.resize(size+7) ;
			std::copy(blockdata.data()+offset,blockdata.data()+offset+7,temp.data()+size);
		}
		offset += 7 ;
	}
	blockdata = temp ;
}
//================================================================================
auto artblock_t::remove(int x, int y,int alt) ->void {
	auto temp = std::vector<std::uint8_t>() ;
	auto xloc = std::uint8_t(0) ;
	auto yloc = std::uint8_t(0) ;
	auto zloc = std::int8_t(0) ;
	auto offset = 0 ;
	while (offset < blockdata.size()){
		std::copy(blockdata.data()+offset+2,blockdata.data()+offset+3,&xloc) ;
		std::copy(blockdata.data()+offset+3,blockdata.data()+offset+4,&yloc) ;
		std::copy(blockdata.data()+offset+4,blockdata.data()+offset+5,reinterpret_cast<std::uint8_t*>(&zloc)) ;
		if ((static_cast<int>(xloc) != x) || (static_cast<int>(yloc) != y) || (static_cast<int>(zloc) != alt)){
			auto size = temp.size() ;
			temp.resize(size+7) ;
			std::copy(blockdata.data()+offset,blockdata.data()+offset+7,temp.data()+size);
		}
		offset += 7 ;
	}
	blockdata = temp ;
}
