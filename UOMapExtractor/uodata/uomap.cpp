//Copyright © 2022 Charles Kerr. All rights reserved.

#include "uomap.hpp"
#include "strutil.hpp"

#include <iostream>
#include <stdexcept>
#include <fstream>

using namespace std::string_literals;

constexpr auto uopblocksize= 4096 ;

static auto entrydata = std::vector<std::uint8_t>(uopblocksize*196,0) ;

//=================================================================================
auto uomap_t::calcBlock(int x, int y) const -> int {
	return ((x/8) * (height/8)) + (y/8) ;
}
//=================================================================================
auto uomap_t::calcXYForBlock(int block) const -> std::pair<int, int> {
	auto x = (block / (height/8)) * 8 ;
	auto y = (block % (height/8)) * 8 ;
	return std::make_pair(x, y);
}
//=================================================================================
auto uomap_t::calcBlockOffset(int x, int y) const -> std::tuple<int, int,int> {
	auto block = calcBlock(x, y) ;
	auto [blockx,blocky] = calcXYForBlock(block);
	auto xloc = x - blockx;
	auto yloc = y - blocky;
	return std::make_tuple(block,xloc,yloc);
}
//  UOP methods

//=================================================================================
auto uomap_t::processEntry(std::size_t entry, std::size_t index, std::vector<std::uint8_t> &data) ->bool {
	//std::cout <<"Process Entry"<< std::endl;
	auto startblock = index*uopblocksize ;
	for (auto i = 0 ; i < (data.size()/196); ++i){
		auto block = i + startblock ;
		if (block< terraindata.size()) {
			std::copy( data.data() + (i*196) , data.data() + (i*196) + 196, terraindata[block].raw().data()) ;
		}
		else {
			break;
		}
	}
	return true ;
}

//=================================================================================
auto uomap_t::entriesToWrite()const ->int {
	return (static_cast<int>(terraindata.size()) /uopblocksize) + ( (terraindata.size()%uopblocksize)!=0?1:0) ;
}
//=================================================================================
auto uomap_t::entryForWrite(int entry)->std::vector<unsigned char>  {
	auto startblock = entry * uopblocksize ;
	std::fill(entrydata.data(),entrydata.data()+entrydata.size(),0) ;
	
	for (auto block = 0  ; block < uopblocksize ; ++block){
		if ((startblock + block) >= terraindata.size() ){
			break;
		}
		std::copy(terraindata[startblock+block].raw().data(),terraindata[startblock+block].raw().data()+196, entrydata.data()+(block*196));
	}
	return entrydata ;
}
//=================================================================================
auto uomap_t::writeHash(int entry)->std::string {
	auto hash = this->format("build/map%ilegacymul/%s", mapnumber,"%.8u.dat");
	return this->format(hash,entry) ;
}

// public

//=================================================================================
uomap_t::uomap_t(int mapnum, int width, int height){
	if (mapnum >= mapsizes.size()) {
		throw std::out_of_range(strutil::format("%i exceeds maximum map size of %i",mapnum, mapsizes.size()-1));
	}
	this->mapnumber = mapnum;
	setSize(width,height) ;
}

//=================================================================================
auto uomap_t::setSize(int width, int height) ->void {
	this->width = width ;
	this->height = height ;
	if ((width == 0) || (height == 0)) {
		const auto &[twidth,theight] = mapsizes[mapnumber];
		this->width = twidth;
		this->height = theight;
	}
	terraindata.resize((this->width/8) * (this->height/8));
	artdata.resize(terraindata.size()) ;
}


//==========================================================================
//   Loading methods
//=========================================================================

//=================================================================================
auto uomap_t::loadTerrainMul(const std::filesystem::path &path) ->bool {
	auto input = std::ifstream(path.string(),std::ios::binary) ;
	auto rvalue = false ;
	auto block = 0 ;
	if (input .is_open()){
		rvalue = true ;
		while (!input.eof() && input.good()) {
			if (block < terraindata.size()) {
				input.read(reinterpret_cast<char*>(terraindata[block].raw().data()),196) ;
				if (input.gcount()!= 196){
					rvalue = false ;
					break;
				}
				++block ;
			}
			else {
				rvalue = false ;
				break;
			}
		}
	}
	return rvalue ;
}

//=================================================================================
auto uomap_t::loadTerrainUOP(const std::filesystem::path &path) ->bool {
	auto hash = this->format("build/map%ilegacymul/%s", mapnumber,"%.8u.dat");
	return loadUOP(path.string(), 0x300, hash);
	
}

//=================================================================================
auto uomap_t::applyTerrainDiff(const std::string &difflpath,const std::string &diffpath) ->bool {
	auto rvalue = false ;
	auto diffl = std::ifstream(difflpath,std::ios::binary) ;
	auto diff = std::ifstream(diffpath,std::ios::binary);
	if (diffl.is_open() && diff.is_open()){
		rvalue = true ;
		auto block = std::uint32_t(0) ;
		while (!diffl.eof() && diffl.good()){
			diffl.read(reinterpret_cast<char*>(&block),4);
			if (diffl.gcount()==4) {
				if (block >= terraindata.size()){
					rvalue = false ;
					break;
				}
				diff.read(reinterpret_cast<char*>(terraindata[block].raw().data()),196);
				if (diff.gcount()!= 196){
					rvalue = false ;
					break;
				}
			}
		}
	}
	return rvalue ;
}

//=================================================================================
auto uomap_t::writeTerrainMul(const std::string &path) const ->bool {
	auto rvalue = false ;
	auto output = std::ofstream(path,std::ios::binary) ;
	if (output.is_open()){
		rvalue = true ;
		for (const auto &block : terraindata){
			output.write(reinterpret_cast<const char*>(block.raw().data()),block.raw().size());
		}
	}
	return rvalue ;
}

//=================================================================================
auto uomap_t::writeTerrainUOP(const std::string &path)  ->bool {
	return writeUOP(path);
}

//=================================================================================
auto uomap_t::loadArt(const std::string &idxpath, const std::string &mulpath) ->bool {
	auto idx = std::ifstream(idxpath,std::ios::binary) ;
	auto mul = std::ifstream(mulpath,std::ios::binary) ;
	auto rvalue = false ;
	if (idx.is_open() && mul.is_open()){
		rvalue = true ;
		auto index = std::uint32_t(0) ;
		auto length = std::uint32_t(0) ;
		auto extra = std::uint32_t(0) ;
		// clear all the statics
		for (auto &artblock:artdata){
			artblock.clear();
		}
		auto block = 0 ;
		while (!idx.eof() && idx.good()){
			idx.read(reinterpret_cast<char*>(&index),4);
			idx.read(reinterpret_cast<char*>(&length),4);
			idx.read(reinterpret_cast<char*>(&extra),4);
			if (idx.gcount()==4){
				if (block >= artdata.size()){
					rvalue = false ;
					break;
				}
				if ((index < 0xFFFFFFFE) && (length >0) && (length < 0xFFFFFFFF)) {
					mul.seekg(index,std::ios::beg) ;
					artdata[block].raw().resize(length,0) ;
					mul.read(reinterpret_cast<char*>(artdata[block].raw().data()),length) ;
					if (mul.gcount()!= length){
						rvalue = false ;
						break;
					}
				}
			}
			block++ ;
		}
	}
	return rvalue ;
}
//=================================================================================
auto uomap_t::applyArtDiff(const std::string &difflpath, const std::string &diffipath, const std::string &diffpath) ->bool {
	auto rvalue = false ;
	auto diffl = std::ifstream(difflpath, std::ios::binary) ;
	auto diffi = std::ifstream(diffipath, std::ios::binary) ;
	auto diff = std::ifstream(diffpath, std::ios::binary) ;
	if (diffl.is_open() && diffi.is_open() && diff.is_open()){
		rvalue = true ;
		auto block = std::uint32_t(0) ;
		auto index = std::uint32_t(0) ;
		auto length = std::uint32_t(0) ;
		auto extra = std::uint32_t(0) ;
		
		while (diffl.good() && !diffl.eof()){
			diffl.read(reinterpret_cast<char*>(&block),4);
			if (diffl.gcount()==4){
				if (block >= artdata.size()) {
					rvalue = false ;
					break;
				}
				diffi.read(reinterpret_cast<char*>(&index),4);
				diffi.read(reinterpret_cast<char*>(&length),4);
				diffi.read(reinterpret_cast<char*>(&extra),4);
				if (diffi.gcount() != 4){
					rvalue = false ;
					break;
				}
				if ((index >= 0xFFFFFFFE ) || (length == 0) || (length >= 0xFFFFFFFF)) {
					artdata[block].clear() ;
				}
				else {
					artdata[block].raw().resize(length,0);
					diff.seekg(index,std::ios::beg);
					diff.read(reinterpret_cast<char*>(artdata[block].raw().data()),length);
					if (diff.gcount()!= length){
						rvalue = false ;
						break;
					}
				}
			}
		}
	}
	return rvalue ;
}

//=================================================================================
auto uomap_t::writeArt(const std::string &idxpath, const std::string &mulpath)const  ->bool {
	auto idx = std::ofstream(idxpath,std::ios::binary) ;
	auto mul = std::ofstream(mulpath,std::ios::binary) ;
	auto rvalue = false ;
	if (idx.is_open()&& mul.is_open()){
		rvalue = true ;
		auto index = std::uint32_t(0) ;
		auto length = std::uint32_t(0) ;
		auto extra = std::uint32_t(0) ;
		for (const auto &block : artdata){
			if (block.size()== 0){
				index = 0xFFFFFFFF;
				length = 0 ;
			}
			else {
				index = static_cast<std::uint32_t>(mul.tellp()) ;
				length = static_cast<std::uint32_t>(block.size());
			}
			idx.write(reinterpret_cast<char*>(&index),4);
			idx.write(reinterpret_cast<char*>(&length),4);
			idx.write(reinterpret_cast<char*>(&extra),4);
			if (length > 0){
				mul.write(reinterpret_cast<const char*>(block.raw().data()),length);
			}
		}
	}
	return rvalue ;
	
}


//=================================================================================
auto uomap_t::terrain(int x, int y) const ->std::pair<std::uint16_t,std::int8_t> {
	auto [block,xoff,yoff] = calcBlockOffset(x, y) ;
	if (block < terraindata.size()) {
		return terraindata[block].terrain(xoff, yoff);
	}
	throw std::out_of_range(strutil::format("Invalid loc(%i,%i), map size %i,%i",x,y,width,height));
}
//=================================================================================
auto uomap_t::terrain(int x, int y, std::uint16_t tileid, std::int8_t altitude) ->void {
	auto [block,xoff,yoff] = calcBlockOffset(x, y) ;
	if (block < terraindata.size()) {
		terraindata[block].terrain(xoff, yoff,tileid,altitude);
	}
	throw std::out_of_range(strutil::format("Invalid loc(%i,%i), map size %i,%i",x,y,width,height));
	
}

//=================================================================================
auto uomap_t::art(int x, int y) const ->std::vector<std::tuple<std::uint16_t,std::int8_t,std::uint16_t>> {
	auto [block,xoff,yoff] = calcBlockOffset(x, y);
	if (block < artdata.size()){
		return artdata[block].art(xoff,yoff) ;
	}
	throw std::out_of_range(strutil::format("Invalid loc(%i,%i), map size %i,%i",x,y,width,height));
	
}
//=================================================================================
auto uomap_t::art(int x, int y, std::uint16_t tileid, std::int8_t altitude, std::uint16_t hue)  ->void {
	auto [block,xoff,yoff] = calcBlockOffset(x, y);
	if (block < artdata.size()){
		artdata[block].art(xoff,yoff,tileid,altitude,hue) ;
	}
	throw std::out_of_range(strutil::format("Invalid loc(%i,%i), map size %i,%i",x,y,width,height));
	
}
//=================================================================================
auto uomap_t::art(int x, int y,int z) const ->std::vector<std::tuple<std::uint16_t,std::int8_t,std::uint16_t>> {
	auto [block,xoff,yoff] = calcBlockOffset(x, y);
	if (block < artdata.size()){
		return artdata[block].art(xoff,yoff,z) ;
	}
	throw std::out_of_range(strutil::format("Invalid loc(%i,%i), map size %i,%i",x,y,width,height));
	
}
//=================================================================================
auto uomap_t::remove(int x, int y) ->void {
	
	auto [block,xoff,yoff] = calcBlockOffset(x, y);
	if (block < artdata.size()){
		artdata[block].remove(xoff,yoff) ;
	}
	throw std::out_of_range(strutil::format("Invalid loc(%i,%i), map size %i,%i",x,y,width,height));
	
}
//=================================================================================
auto uomap_t::remove(int x, int y, int z) ->void {
	auto [block,xoff,yoff] = calcBlockOffset(x, y);
	if (block < artdata.size()){
		artdata[block].remove(xoff,yoff,z) ;
	}
	throw std::out_of_range(strutil::format("Invalid loc(%i,%i), map size %i,%i",x,y,width,height));
}

