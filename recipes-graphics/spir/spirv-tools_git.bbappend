# Update default repo branches from 'master' to 'main' since github
# is using the latter one.
python () {
    import re
    uri = d.getVar("SRC_URI", True)
    uri_updated = re.sub("branch=master", "branch=main", uri)
    d.setVar("SRC_URI", uri_updated)
}
