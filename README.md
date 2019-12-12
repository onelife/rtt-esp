# rtt-esp

- Start docker
  - cd tools/docker
  - docker-compose up -d
  - docker ps
  - docker attach xxx

- Inside container
  - cd ./rtt/hello_world
  - . $IDF_PATH/export.sh
  - idf.py build
