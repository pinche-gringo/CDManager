# Base image for the CI-jobs of CDManager (see .gitlab-ci.yml)
#
# Changing this file rebuilds the image on the next push.
# libYGP/libXGP aren't included; the jobs install the needed version themselves.
FROM fedora:45

RUN dnf install -y --setopt=install_weak_deps=False \
        gcc gcc-c++ meson ninja-build git gettext gettext-devel rpm-build \
        boost-devel gtkmm4.0-devel libsoup3-devel libpq-devel \
        curl \
    && dnf clean all
