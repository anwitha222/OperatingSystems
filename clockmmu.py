from mmu import MMU


class ClockMMU(MMU):
    def __init__(self, frames):
        # TODO: Constructor logic for clockMMU
        # configure frames
        self.frames = frames
        # debug flag
        self.debug = False

        # set up counters
        self.diskReads = 0
        self.diskWrites = 0
        self.pageFaults = 0

        # initialise the frame table {"page": X, "dirty": bool, "use": 0 or 1})
        self.frame_table = [None] * frames

        # map page to frame index to achieve O(1) hits
        self.page_to_frame = {}

        # list of free frames to use before any eviction
        self.free_frames = list(range(frames))

        # clock hand pointer
        self.clock_hand = 0

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
    
    def _access(self, page_number, write):
        # in the case of a hit
        if page_number in self.page_to_frame:
            # get the frame and slot
            frame = self.page_to_frame[page_number]
            slot = self.frame_table[frame]

            # if it is a write, set the bit dirty
            if write and not slot["dirty"]:
                slot["dirty"] = True
            
            # set use bit to 1 regardless of read or write
            slot["use"] = 1

            # debugging info
            if self.debug:
                op = "W" if write else "R"
                print(f"hit: page {page_number} in frame {frame} ({op}) dirty = {slot['dirty']} use = {slot['use']}")
            return
        
        # in the case of a miss
        self.pageFaults += 1
        self.diskReads += 1

        # use a free frame if available
        if self.free_frames:
            frame = self.free_frames.pop(0) # get the first free frame
            self._install_page(frame, page_number, write) # install the page

             # debugging info
            if self.debug:
                op = "W" if write else "R"
                print(f"miss: load page {page_number} into free frame {frame} ({op})")
            return

        # if there is no free frame, we must evict a page
        while (True):
            slot = self.frame_table[self.clock_hand]
            
            if slot["use"] == 1:
                # give the page a second chance
                slot["use"] = 0

                if self.debug:
                    print(f"second chance: page {slot['page']} in frame {self.clock_hand} use = {slot['use']}")
                
                # move the clock hand to the next frame
                self.clock_hand = (self.clock_hand + 1) % self.frames

            else:
                # evict the victim page
                victim_page = slot["page"]
                if slot["dirty"]:
                    self.diskWrites += 1 # write the page back to disk if dirty
                    if self.debug:
                        print(f"evicting dirty page {victim_page} from frame {self.clock_hand}, (dirty->write back)")
                        
                # remove the mapping of the victim page
                del self.page_to_frame[victim_page]

                # install the new page in the victim frame
                self._install_page(self.clock_hand, page_number, write)

                if self.debug:
                    op = "W" if write else "R"
                    print(f"miss occured, load page {page_number} into frame {self.clock_hand} ({op})")

                # move the clock hand to the next frame
                self.clock_hand = (self.clock_hand + 1) % self.frames
                break

    def _install_page(self, frame, page_number, is_written):
        # place page in frame and update the structures
        self.frame_table[frame] = {
            "page": page_number,
            "dirty": bool(is_written),
            "use": 1, # set use bit to 1 when installing a new page
        }
        self.page_to_frame[page_number] = frame # map page to frame




