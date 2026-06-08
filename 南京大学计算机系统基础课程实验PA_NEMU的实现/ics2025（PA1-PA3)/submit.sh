COURSE=ICS2025
MODULE=$(git rev-parse --abbrev-ref HEAD | tr '[a-z]' '[A-Z]')
FILE=/tmp/upload.tar.bz2
tar caf "$FILE" ".git" $(find . -maxdepth 2 -name "*.pdf") && \
  curl -F token=$TOKEN \
       -F course=$COURSE \
       -F module=$MODULE \
       -F file=@$FILE \
       http://118.89.179.200:8080/upload
