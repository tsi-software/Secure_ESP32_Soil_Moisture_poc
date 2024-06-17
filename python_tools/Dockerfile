FROM python:3.11-slim
#FROM python:3.11
RUN python3 -m pip install --upgrade pip

WORKDIR /python_tools
COPY requirements.txt  /python_tools/
RUN python3 -m pip install --requirement requirements.txt

COPY *.sh *.py *.ini *.txt  /python_tools/

#ENTRYPOINT ["python3", "/python_tools/watch_selected_data.py"]
CMD ["bash"]
