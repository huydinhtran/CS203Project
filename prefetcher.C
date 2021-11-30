#include <stdio.h>
#include <math.h>
#include "prefetcher.h"
#include "mem-sim.h"
#include "cache.h"
#include "memQueue.h"
#include "CPU.h"

Prefetcher pf;
Request req;

Prefetcher rpt[4096];

int entryCount = 0;

int old_stride=0;
int old_prev_addr=0;
bool first = true;


Prefetcher::Prefetcher() { 
	tag=0;
	prev_addr=0;
	stride=0;
	state=0;
	_ready = false; 
}

bool Prefetcher::hasRequest(u_int32_t cycle) {

	return _ready;

}

Request Prefetcher::getRequest(u_int32_t cycle) {

	Request req;
	return _nextReq;

}

void Prefetcher::completeRequest(u_int32_t cycle) { 

	_ready = false;

}

void Prefetcher::cpuRequest(Request req) {

	int tag_check=pf.findTag(pf.getTag(req.addr));

	if (entryCount == 0 && first == true) {
		rpt[0].tag       = pf.getTag(req.addr);
		rpt[0].prev_addr = req.addr;
		rpt[0].stride    = 0;
		rpt[0].state     = 0;
		entryCount++;
		first = false;
	}

	if (entryCount != 0 && tag_check == 5000) {	
		rpt[entryCount % 4096].tag       = pf.getTag(req.addr);
		rpt[entryCount % 4096].prev_addr = req.addr;
		rpt[entryCount % 4096].stride    = req.addr - prev_addr;
		rpt[entryCount % 4096].state     = 0;
		entryCount++;

	}else{
		rpt[tag_check].tag       = pf.getTag(req.addr);
		old_prev_addr            = rpt[tag_check].prev_addr;
		rpt[tag_check].prev_addr = req.addr;
		old_stride               = rpt[tag_check].stride;
		rpt[tag_check].stride    = req.addr - old_prev_addr;
		
		if (rpt[tag_check].state == 0 && rpt[tag_check].stride==old_stride){
			rpt[tag_check].state = 2;
			_nextReq.addr = rpt[tag_check].prev_addr + rpt[tag_check].stride + 16+16+16+2;
			_ready = true;

		}

		if (rpt[tag_check].state == 0 && rpt[tag_check].stride!=old_stride){
			rpt[tag_check].state = 1;
		}

		if (rpt[tag_check].state == 1 && rpt[tag_check].stride==old_stride){
			rpt[tag_check].state = 2;
			_nextReq.addr = rpt[tag_check].prev_addr + rpt[tag_check].stride + 16+16+16+2;
			_ready = true;

		}

		if (rpt[tag_check].state == 1 && rpt[tag_check].stride!=old_stride){
			rpt[tag_check].state = 3;
		}

		if (rpt[tag_check].state == 2 && rpt[tag_check].stride==old_stride){
			rpt[tag_check].state = 2;
			_nextReq.addr = rpt[tag_check].prev_addr + rpt[tag_check].stride + 16+16+16+2;
			_ready = true;
		}

		if (rpt[tag_check].state == 2 && rpt[tag_check].stride!=old_stride){
			rpt[tag_check].state = 0;
		}

		if (rpt[tag_check].state == 3 && rpt[tag_check].stride==old_stride){
			rpt[tag_check].state = 2;
			_nextReq.addr = rpt[tag_check].prev_addr + rpt[tag_check].stride + 16+16+16+2;
			_ready = true;

		}

		if (rpt[tag_check].state == 3 && rpt[tag_check].stride!=old_stride){
			rpt[tag_check].state = 3;
		}
	}
}

u_int32_t Prefetcher::getTag(u_int32_t addr) {

	int b = (int)log(32)/log(2);
	int s = (int)log(256 * 4096 / (8 * 32))/log(2);
	int t = 32 - b - s;

	u_int32_t tag = addr >> (b + s);
	return tag;

}

u_int32_t Prefetcher::findTag(u_int32_t value) {

    int index = 0;

    while ( index < 4096 && rpt[index].tag != value ) ++index;

    return ( index == 4096 ? 5000 : index );
	
}
