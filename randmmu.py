from mmu import MMU
import random 

class RandMMU(MMU):
    def __init__(self, frames):
        # TODO: Constructor logic for RandMMU
        
        # core configuration
        self.frames = frames
        # set debug flag
        self.debug = False

        # initialise counters
        self.diskReads = 0
        self.diskWrites = 0
        self.pageFaults = 0

        # frame table: list of dicts or none and size = frames
        # for each occupied slot: "Page": int, "dirty": bool
        self.frame_table = [None] * frames

        # map page to frame index for O(1) hits
        self.page_to_frame = {}

        # list of free frames to use before eviction
        self.free_frames = list(range(frames))



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
        # hit?
        if page_number in self.page_to_frame: 
            frame = self.page_to_frame[page_number] # get the frame index
            slot = self.frame_table[frame] # get the slot dict
            # on write, mark dirty
            if write and not slot["dirty"]:
                slot["dirty"] = True

            # debugging info
            if self.debug:
                op = "W" if write else "R"
                print(f"hit: page {page_number} in frame {frame} ({op}) dirty = {slot['dirty']}")
            return
        
        # miss: page fault
        self.pageFaults += 1 # increment page faults
        self.diskReads += 1 # read page from disk

        #use a free frame if available
        if self.free_frames:
            frame = self.free_frames.pop(0) # get the first free frame

            # install the page
            self._install_page(frame, page_number, write)
            if self.debug:
                op = "W" if write else "R"
                print(f"miss: load page {page_number} into free frame {frame} ({op})")
            return
        
        #no free frames: evict a random frame
        victim_frame = random.randrange(self.frames) # pick a random frame as victim
        victim_slot = self.frame_table[victim_frame] # get the slot dict
        victim_page = victim_slot["page"] # get the victims page number

        #if victim dirty: write back
        if victim_slot["dirty"]:
            self.diskWrites += 1 # increment disk writes
            if self.debug:
                print(f"evict: page {victim_page} from frame {victim_frame} (dirty->write back)")

        #remove victim mapping
        del self.page_to_frame[victim_page]

        #install new page in victim frame
        self._install_page(victim_frame, page_number, write)
        if self.debug:
            op = "W" if write else "R"
            print(f"miss: load page {page_number} into frame {victim_frame} ({op})")

    def _install_page(self, frame, page_number, write):
        #place page in frame and update maps
        self.frame_table[frame] = {
            "page":page_number, 
            "dirty": bool(write),
        }
        self.page_to_frame[page_number] = frame # map page to frame
                

