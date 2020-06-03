FROM jhunkeler/spm_ci_centos7:latest
ARG RELOC_COMMIT=${RELOC_COMMIT:-}
ARG SPM_COMMIT=${SPM_COMMIT:-}
ENV PATH /opt/spm/bin:/usr/bin:/usr/sbin:/bin:/sbin
ENV SHELL /bin/bash

RUN ln -s cmake3 /usr/bin/cmake \
	&& git clone https://github.com/jhunkeler/spmc \
	&& git clone https://github.com/jhunkeler/reloc \
	&& pushd reloc && mkdir build && cd build && git checkout ${RELOC_COMMIT} && cmake .. -DCMAKE_INSTALL_PREFIX=/opt/spm && make install && popd \
	&& pushd spmc && mkdir build && cd build && git checkout ${SPM_COMMIT} && cmake .. -DCMAKE_INSTALL_PREFIX=/opt/spm && make install && popd

WORKDIR /spm_packages
CMD ["bash", "-l"]
