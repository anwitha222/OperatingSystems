from mmu import MMU
import random 

class RandMMU(MMU):
    def __init__(self, frames):
        # TODO: Constructor logic for RandMMU
        
        #core configuration
        self.frames = frames
        #debug flag
        self.debug = False

        #counters
        self.disk_reads = 0
        self.disk_writes = 0
        self.page_faults = 0

        #frame table: list of dicts or none and size = frames
        #for each occupied slot: "Page": int, "dirty": bool
        self.frame_table = [None] * frames

        #map page to frame index for O(1) hits
        self.page_to_frames = list(range(frames))



    def set_debug(self):
        # TODO: Implement the method to set debug mode
        self.debug = True

    def reset_debug(self):
        # TODO: Implement the method to reset debug mode
        self.debug = False

    def read_memory(self, page_number):
        # TODO: Implement the method to read memory
        self._access(page_number, is_write = False)

    def write_memory(self, page_number):
        # TODO: Implement the method to write memory
        self._access(page_number, is_write = True)

    def get_total_disk_reads(self):
        # TODO: Implement the method to get total disk reads
        return self.disk_reads

    def get_total_disk_writes(self):
        # TODO: Implement the method to get total disk writes
        return self.disk_writes

    def get_total_page_faults(self):
        # TODO: Implement the method to get total page faults
        return self.page_faults
    

    #helpers 

    def _access(self, page_number, is_write):
        #hit?
        if page_number in self.page_to_frame:
            frame = self.page_to_frame[page_number]
            slot = self.frame_table[frame]
            #on write, mark dirty
            if is_write and not slot["dirty"]:
                slot["dirty"] = True
            if self.debug:
                op = "W" if is_write else "R"
                print(f"hit: page {page_number} in frame {frame} ({op}) dirty = {slot['dirty']}")
            return
        
        #miss: page fault
        self.page_faults += 1
        self.disk_reads += 1 #read page from disk

        #use a free frame if available
        if self.free_frames:
            frame = self.free_frames.pop(0)
            self._install_page(frame, page_number, is_write)
            if self.debug:
                op = "W" if is_write else "R"
                print(f"miss: load page {page_number} into free frame {frame} ({op})")
            return
        
        #no free frames: evict a random variable
        victim_frame = random.randrange(self.frames)
        victim_slot = self.frame_table[victim_frame]
        victim_page = victim_slot["page"]

        #if victim dirty: write back
        if victim_slot["dirty"]:
            self.disk_writes += 1
            if self.debug:
                print(f"evict: page {victim_page} from frame {victim_frame} (dirty->write back)")

            #remove victim mapping
            del self.page_to_frame[victim_page]

            #install new page in victim frame
            self._install_page(victim_frame, page_number, is_write)
            if self.debug:
                op = "W" if is_write else "R"
                print(f"miss: load page {page_number} into frame {victim_frame} ({op})")

            def _install_page(self, frame, page_number, is_write):
                #place page in frame and update maps
                self.frame_table[frame] = {
                    "page":page_number, 
                    "dirty": bool(is_write),
                }
                self.page_to_frame[page_number] = frame
                

