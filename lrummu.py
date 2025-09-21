from mmu import MMU
from collections import OrderedDict

class LruMMU(MMU):
    def __init__(self, frames):
        # TODO: Constructor logic for LruMMU
        # configure frames
        self.frames = frames
        #  set debug flag
        self.debug = False

        # counters set up
        self.diskReads = 0
        self.diskWrites = 0
        self.pageFaults = 0

        # frame table
        self.frame_table = [None] * frames

        # fast lookup: page -> frame index
        self.page_to_frame = {}

        # free frames to use before any eviction
        self.free_frames = list(range(frames))

        # least recently used order: keys are pages, most recently used at the end
        self.lru = OrderedDict()


    def set_debug(self):
        # TODO: Implement the method to set debug mode
        self.debug = True

    def reset_debug(self):
        # TODO: Implement the method to reset debug mode
        self.debug = False

    def read_memory(self, page_number):
        # TODO: Implement the method to read memory
        self._access(page_number, write = False)


    def write_memory(self, page_number):
        # TODO: Implement the method to write memory
        self._access(page_number, write = True)

    def get_total_disk_reads(self):
        # TODO: Implement the method to get total disk reads
        return self.diskReads

    def get_total_disk_writes(self):
        # TODO: Implement the method to get total disk writes
        return self.diskWrites

    def get_total_page_faults(self):
        # TODO: Implement the method to get total page faults
        return self.pageFaults
    
    # helper functions

    def _access(self, page_number, write):
        # in the case of a hit
        if page_number in self.page_to_frame:
            frame = self.page_to_frame[page_number] # get the frame index
            slot = self.frame_table[frame] # get the slot dict

            # mark dirty if it is a write
            if write and not slot ["dirty"]:
                slot["dirty"] = True

            #refresh lru: move to mru
            if page_number in self.lru:
                self.lru.move_to_end(page_number, last = True) # most recently used
            else:
                self.lru[page_number] = True # add to lru if not present

            # debug info
            if self.debug: 
                op = "W" if write else "R"
                print(f"hit: page {page_number} in frame {frame} ({op}) dirty = {slot['dirty']}")
            return
        
        # in the case of a miss
        self.pageFaults += 1 # increment page faults
        self.diskReads += 1 # read page from disk

        # use free frame is there is one
        if self.free_frames: 
            frame = self.free_frames.pop(0) # get the first free frame
            self._install_page(frame, page_number, write) # install the page
            if self.debug:
                op = "W" if write else "R"
                print(f"miss: load page {page_number} into free frame {frame} ({op})")
            return 
        
        # evict lru at the front of ordered dict
        victim_page, _ = self.lru.popitem(last = False) # get lru page
        victim_frame = self.page_to_frame.pop(victim_page) # get the frame index
        victim_slot = self.frame_table[victim_frame] # get the slot dict

        if victim_slot["dirty"]:
            self.diskWrites += 1
            if self.debug:
                print(f"evict: page {victim_page} from frame {victim_frame} (dirty->write back)")

        #install new page in empty frame
        self._install_page(victim_frame, page_number, write)
        if self.debug: 
            op = "W" if write else "R"
            print(f"miss: load page {page_number} into frame {victim_frame} ({op})")

    def _install_page(self, frame, page_number, write):
        #place page in frame and update the structures
        self.frame_table[frame] = {
            "page": page_number, 
            "dirty": bool(write),
        }

        self.page_to_frame[page_number] = frame # map page to frame
        self.lru[page_number] = True # add to lru
        self.lru.move_to_end(page_number, last=True) # set as most recently used 

