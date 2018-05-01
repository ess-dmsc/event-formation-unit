project = "event-formation-unit"
coverage_on = "centos7"

images = [
    'centos7': [
        'name': 'essdmscdm/centos7-build-node:1.0.1',
        'sh': 'sh'
    ],
//    'centos7-release': [
//        'name': 'essdmscdm/centos7-build-node:1.0.1',
//        'sh': 'sh'
//    ],
    'centos7-gcc6': [
        'name': 'essdmscdm/centos7-gcc6-build-node:2.1.0',
        'sh': '/usr/bin/scl enable rh-python35 devtoolset-6 -- /bin/bash'
    ],
    'fedora25': [
        'name': 'essdmscdm/fedora25-build-node:1.0.0',
        'sh': 'sh'
    ],
    'ubuntu1604': [
        'name': 'essdmscdm/ubuntu16.04-build-node:2.1.0',
        'sh': 'sh'
    ],
    'ubuntu1710': [
        'name': 'essdmscdm/ubuntu17.10-build-node:2.0.0',
        'sh': 'sh'
    ]
]

base_container_name = "${project}-${env.BRANCH_NAME}-${env.BUILD_NUMBER}"

def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.',
            recipientProviders: toEmails,
            subject: '${DEFAULT_SUBJECT}'
    slackSend color: 'danger',
            message: "${project}-${env.BRANCH_NAME}: " + failureMessage
    throw exception_obj
}

def Object container_name(image_key) {
    return "${base_container_name}-${image_key}"
}

def Object get_container(image_key) {
    def image = docker.image(images[image_key]['name'])
    def container = image.run("\
        --name ${container_name(image_key)} \
        --tty \
        --env http_proxy=${env.http_proxy} \
        --env https_proxy=${env.https_proxy} \
        --env local_conan_server=${env.local_conan_server} \
        ")
    return container
}

def docker_clone(image_key) {
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        git clone \
            --branch ${env.BRANCH_NAME} \
            https://github.com/ess-dmsc/event-formation-unit.git /home/jenkins/${project}
    \""""
}

def docker_dependencies(image_key) {
    def conan_remote = "ess-dmsc-local"
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        mkdir ${project}/build
        cd ${project}/build
        conan remote add \
            --insert 0 \
            ${conan_remote} ${local_conan_server}
    \""""
}

def docker_cmake(image_key, xtra_flags) {
    cmake_exec = "cmake"
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        cd ${project}/build
        ${cmake_exec} --version
        ${cmake_exec} ${xtra_flags} ..
    \""""
}

def docker_build(image_key) {
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        cd ${project}/build
        . ./activate_run.sh
        make --version
        make VERBOSE=ON
    \""""
}

def docker_tests(image_key) {
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        cd ${project}/build
        . ./activate_run.sh
        make runtest VERBOSE=ON
        make runefu VERBOSE=ON
    \""""
}

def docker_tests_coverage(image_key) {
    def custom_sh = images[image_key]['sh']
    abs_dir = pwd()

    try {
        sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
                cd ${project}/build
                . ./activate_run.sh
                make VERBOSE=ON
                make runefu VERBOSE=ON
                make coverage VERBOSE=ON
                make valgrind VERBOSE=ON
            \""""
        sh "docker cp ${container_name(image_key)}:/home/jenkins/${project} ./"
    } catch(e) {
        sh "docker cp ${container_name(image_key)}:/home/jenkins/${project}/build/test_results ./"
        junit 'test_results/*.xml'
        failure_function(e, 'Run tests (${container_name(image_key)}) failed')
    }

    dir("${project}/build") {
            junit 'test_results/*.xml'
            sh "../jenkins/redirect_coverage.sh ./coverage/coverage.xml ${abs_dir}/${project}"

            step([
                $class: 'CoberturaPublisher',
                autoUpdateHealth: true,
                autoUpdateStability: true,
                coberturaReportFile: 'coverage/coverage.xml',
                failUnhealthy: false,
                failUnstable: false,
                maxNumberOfBuilds: 0,
                onlyStable: false,
                sourceEncoding: 'ASCII',
                zoomCoverageChart: true
            ])
            step([$class: 'ValgrindPublisher',
                  pattern: 'memcheck_res/*.valgrind',
                  failBuildOnMissingReports: true,
                  failBuildOnInvalidReports: true,
                  publishResultsForAbortedBuilds: false,
                  publishResultsForFailedBuilds: false,
                  failThresholdInvalidReadWrite: '',
                  unstableThresholdInvalidReadWrite: '',
                  failThresholdDefinitelyLost: '',
                  unstableThresholdDefinitelyLost: '',
                  failThresholdTotal: '',
                  unstableThresholdTotal: '99'
            ])
            //archiveArtifacts artifacts: 'build/'
    }
}

def get_pipeline(image_key)
{
    return {
        stage("${image_key}") {
            node ("docker") {
                try {
                    def container = get_container(image_key)

                    docker_clone(image_key)
                    docker_dependencies(image_key)

                    if (image_key == coverage_on) {
                        docker_cmake(image_key, "-DDUMPTOFILE=ON -DCOV=1")
                    } else {
                        docker_cmake(image_key, "-DDUMPTOFILE=ON")
                    }

                    docker_build(image_key)

                    if (image_key == coverage_on) {
                        docker_tests_coverage(image_key)
                    } else {
                        docker_tests(image_key)
                    }
                } finally {
                    sh "docker stop ${container_name(image_key)}"
                    sh "docker rm -f ${container_name(image_key)}"
                }
            }
        }
    }
}

def get_macos_pipeline()
{
    return {
        stage("macOS") {
            node ("macos") {
            // Delete workspace when build is done
                cleanWs()

                dir("${project}/code") {
                    checkout scm
                }

                dir("${project}/build") {
                    sh "cmake -DDUMPTOFILE=ON -DCMAKE_MACOSX_RPATH=ON ../code"
                    sh "make"
                    sh "make runtest"
                }
            }
        }
    }
}

def get_release_pipeline()
{
    // Build with release settings to archive artefacts.
    return {
        stage("release-centos7") {
            node('docker') {
                def container_name = "${base_container_name}-release-centos7"
                try {
                    def image = docker.image("essdmscdm/centos7-build-node:1.0.1")
                    def container = image.run("\
                        --name ${container_name} \
                        --tty \
                        --env http_proxy=${env.http_proxy} \
                        --env https_proxy=${env.https_proxy} \
                        --env local_conan_server=${env.local_conan_server} \
                        ")
                    def custom_sh = "sh"
                    def conan_remote = "ess-dmsc-local"

                    sh """docker exec ${container_name} ${custom_sh} -c \"
                        git clone \
                            --branch ${env.BRANCH_NAME} \
                            https://github.com/ess-dmsc/event-formation-unit.git \
                            ${project}
                    \""""

                    sh """docker exec -u root ${container_name} ${custom_sh} -c \"
                        yum install -y libpcap-devel
                    \""""

                    sh """docker exec ${container_name} ${custom_sh} -c \"
                        mkdir build && \
                        cd build && \
                        conan remote add \
                            --insert 0 \
                            ${conan_remote} ${local_conan_server} && \
                    \""""

                    sh """docker exec ${container_name} ${custom_sh} -c \"
                        cd ${project} && \
                        BUILDSTR=\\\$(git log --oneline | head -n 1 | awk '{print \\\$1}') && \
                        cd ../build && \
                        cmake --version && \
                        cmake \
                            -DCMAKE_BUILD_TYPE=Release \
                            -DCMAKE_SKIP_BUILD_RPATH=ON \
                            -DBUILDSTR=\\\$BUILDSTR \
                            ../${project}
                    \""""

                    sh """docker exec ${container_name} ${custom_sh} -c \"
                        cd build && \
                        . ./activate_run.sh && \
                        make VERBOSE=ON -j4 && \
                        make VERBOSE=ON -j4 runtest && \
                        make VERBOSE=ON -j4 runefu
                    \""""

                    sh """docker exec ${container_name} ${custom_sh} -c \"
                        mkdir -p archive/event-formation-unit && \
                        cp -r build/bin archive/event-formation-unit && \
                        cp -r build/lib archive/event-formation-unit && \
                        cp -r build/licenses archive/event-formation-unit && \
                        mkdir archive/event-formation-unit/util && \
                        cp -r ${project}/utils/efushell archive/event-formation-unit/util && \
                        cp -r ${project}/monitors/* archive/event-formation-unit/util && \
                        mkdir archive/event-formation-unit/data && \
                        cp -r ${project}/prototype2/multigrid/calib_data/* archive/event-formation-unit/data && \
                        cd archive && \
                        tar czvf event-formation-unit-centos7.tar.gz event-formation-unit
                    \""""

                    sh "docker cp ${container_name}:/home/jenkins/archive/event-formation-unit-centos7.tar.gz ."
                    archiveArtifacts "event-formation-unit-centos7.tar.gz"
                } finally {
                    sh "docker stop ${container_name}"
                    sh "docker rm -f ${container_name}"
                }
            }
        }
    }
}

node('docker') {

    // Delete workspace when build is done
    cleanWs()

    dir("${project}_code") {

        stage('Checkout') {
            try {
                scm_vars = checkout scm
            } catch (e) {
                failure_function(e, 'Checkout failed')
            }
        }

        stage("Static analysis") {
            try {
                sh "cloc --by-file --xml --out=cloc.xml ."
                sh "xsltproc jenkins/cloc2sloccount.xsl cloc.xml > sloccount.sc"
                sloccountPublish encoding: '', pattern: ''
            } catch (e) {
                failure_function(e, 'Static analysis failed')
            }
        }
    }

    def builders = [:]

    for (x in images.keySet()) {
        def image_key = x
        builders[image_key] = get_pipeline(image_key)
    }
    builders['macOS'] = get_macos_pipeline()
    builders['release-centos7'] = get_release_pipeline()

    try {
        parallel builders
    } catch (e) {
        failure_function(e, 'Job failed')
    }
}
