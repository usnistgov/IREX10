
subDirs = stub validation 

all: $(subDirs)

$(subDirs):
	$(MAKE) -C $@

clean: $(subDirs)
	for d in $(subDirs); do \
	  $(MAKE) -C $$d  $@; \
	done

.PHONY: clean $(subDirs)

