FROM python:3.11-slim
#FROM python:3.11
RUN python3 -m pip install --upgrade pip

COPY *.sh *.py *.ini *.txt   /soilmoisture_listener/

WORKDIR /soilmoisture_listener
RUN python3 -m pip install --requirement requirements.txt

ENTRYPOINT ["python3", "/soilmoisture_listener/soilmoisture_listener.py"]


# To Build:
#docker build --rm --pull --tag soilmoisture_listener:v0.0.1 .

# To Run:
#docker run --rm --name soil --detach --volume ../../private:/soilmoisture_listener/private soilmoisture_listener:v0.0.1 && docker container logs soil --follow

# To gain shell access:
#docker exec -it soil /bin/bash
