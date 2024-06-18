FROM espressif/idf:release-v5.2
#FROM espressif/idf:latest

#RUN pip3 install --upgrade pip
#RUN pip3 install mkdocs

#-------------------------------------------------------------------------------
# From:
# https://github.com/docker-library/postgres/blob/69bc540ecfffecce72d49fa7e4a46680350037f9/9.6/Dockerfile#L21-L24
#-------------------------------------------------------------------------------
# Make the "en_US.UTF-8" locale so the image will be utf-8 enabled by default.
#RUN apt-get update  \
#  && apt-get install -y locales  \
#  && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
#ENV LANG en_US.utf8

#-------------------------------------------------------------------------------
# OR AN ALTERNATE APPROACH:
# Install locales
# Uncomment en_CA.UTF-8 for inclusion in generation
# Generate locale
RUN apt-get update  \
  && apt-get install -y locales  \
  && sed -i 's/^# *\(en_CA.UTF-8\)/\1/' /etc/locale.gen  \
  && locale-gen
