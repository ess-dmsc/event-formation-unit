project = "event-formation-unit"
coverage_on = "centos7"
clangformat_os = "fedora25"
archive_what = "centos7-release"

// Set number of old builds to keep.
 properties([[
     $class: 'BuildDiscarderProperty',
     strategy: [
         $class: 'LogRotator',
         artifactDaysToKeepStr: '',
         artifactNumToKeepStr: '10',
         daysToKeepStr: '',
         numToKeepStr: ''
     ]
 ]]);

images = [
    'centos7-release': [
        'name': 'essdmscdm/centos7-build-node:3.2.0',
        'sh': '/usr/bin/scl enable rh-python35 devtoolset-6 -- /bin/bash -e',
        'cmake_flags': '-DCMAKE_BUILD_TYPE=Release -DCMAKE_SKIP_BUILD_RPATH=ON'
    ],
    'centos7': [
        'name': 'essdmscdm/centos7-build-node:3.2.0',
        'sh': '/usr/bin/scl enable rh-python35 devtoolset-6 -- /bin/bash -e',
        'cmake_flags': '-DCOV=ON'
    ],
    'ubuntu1804': [
        'name': 'essdmscdm/ubuntu18.04-build-node:1.2.0',
        'sh': 'bash -e',
        'cmake_flags': ''
    ],
    'fedora25': [
        'name': 'essdmscdm/fedora25-build-node:2.0.0',
        'sh'  : 'bash -e',
        'cmake_flags': ''
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

def docker_copy_code(image_key) {
    def custom_sh = images[image_key]['sh']
    sh "docker cp ${project}_code ${container_name(image_key)}:/home/jenkins/${project}"
    sh """docker exec --user root ${container_name(image_key)} ${custom_sh} -c \"
                        chown -R jenkins.jenkins /home/jenkins/${project}
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
        conan install --build=outdated ..
    \""""
}

def docker_cmake(image_key, xtra_flags) {
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        cd ${project}
        cd build
        . ./activate_run.sh
        cmake --version
        cmake -DCONAN=MANUAL -DGOOGLE_BENCHMARK=ON ${xtra_flags} ..
    \""""
}

def docker_build(image_key) {
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        cd ${project}/build
        make --version
        make -j4 VERBOSE=OFF
        make -j4 unit_tests VERBOSE=OFF
        make -j4 benchmark
        cd ../utils/udpredirect
        make
    \""""
}

def docker_cppcheck(image_key) {
    try {
        def custom_sh = images[image_key]['sh']
        def test_output = "cppcheck.txt"
        def cppcheck_script = """
                        cd ${project}
                        cppcheck --enable=all --inconclusive --template="{file},{line},{severity},{id},{message}" ./ 2> ${test_output}
                    """
        sh "docker exec ${container_name(image_key)} ${custom_sh} -c \"${cppcheck_script}\""
        sh "docker cp ${container_name(image_key)}:/home/jenkins/${project} ."
        sh "mv -f ./${project}/* ./"
    } catch (e) {
        failure_function(e, "Cppcheck step for (${container_name(image_key)}) failed")
    }
}

def docker_tests(image_key) {
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        cd ${project}/build
        . ./activate_run.sh
        make runtest
        make runefu
    \""""
}

def docker_tests_coverage(image_key) {
    def custom_sh = images[image_key]['sh']
    abs_dir = pwd()

    try {
        sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
                cd ${project}/build
                . ./activate_run.sh
                make -j 4 runefu
                make coverage
                echo skipping make -j 4 valgrind
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
            // step([$class: 'ValgrindPublisher',
            //      pattern: 'memcheck_res/*.valgrind',
            //      failBuildOnMissingReports: true,
            //      failBuildOnInvalidReports: true,
            //      publishResultsForAbortedBuilds: false,
            //      publishResultsForFailedBuilds: false,
            //      failThresholdInvalidReadWrite: '',
            //      unstableThresholdInvalidReadWrite: '',
            //      failThresholdDefinitelyLost: '',
            //      unstableThresholdDefinitelyLost: '',
            //      failThresholdTotal: '',
            //      unstableThresholdTotal: '99'
            //])
    }
}

def docker_archive(image_key) {
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
                        mkdir -p archive/event-formation-unit
                        cp -r ${project}/build/bin archive/event-formation-unit
                        cp -r ${project}/build/modules archive/event-formation-unit
                        cp -r ${project}/build/lib archive/event-formation-unit
                        cp -r ${project}/build/licenses archive/event-formation-unit
                        mkdir archive/event-formation-unit/util
                        cp -r ${project}/utils/efushell archive/event-formation-unit/util
                        mkdir archive/event-formation-unit/configs
                        cp -r ${project}/prototype2/multiblade/configs/* archive/event-formation-unit/configs/
                        cp -r ${project}/prototype2/gdgem/configs/* archive/event-formation-unit/configs/
                        cp ${project}/utils/udpredirect/udpredirect archive/event-formation-unit/util
                        cp -r ${project}/utils/hwcheck archive/event-formation-unit/util/
                        cp -r ${project}/monitors/* archive/event-formation-unit/util
                        mkdir archive/event-formation-unit/data
                        cp -r ${project}/prototype2/multigrid/calib_data/* archive/event-formation-unit/data
                        cd archive
                        tar czvf event-formation-unit-centos7.tar.gz event-formation-unit

                        # Create file with build information
                        touch BUILD_INFO
                        echo 'Repository: ${project}/${env.BRANCH_NAME}' >> BUILD_INFO
                        echo 'Commit: ${scm_vars.GIT_COMMIT}' >> BUILD_INFO
                        echo 'Jenkins build: ${BUILD_NUMBER}' >> BUILD_INFO
                    \""""

    sh "docker cp ${container_name(image_key)}:/home/jenkins/archive/event-formation-unit-centos7.tar.gz ."
    sh "docker cp ${container_name(image_key)}:/home/jenkins/archive/BUILD_INFO ."
    archiveArtifacts "event-formation-unit-centos7.tar.gz,BUILD_INFO"
}

def get_pipeline(image_key)
{
    return {
        stage("${image_key}") {
            try {
                def container = get_container(image_key)

                docker_copy_code(image_key)
                if (image_key != clangformat_os) {
                  docker_dependencies(image_key)
                  docker_cmake(image_key, images[image_key]['cmake_flags'])
                  docker_build(image_key)
                }

                if (image_key == coverage_on) {
                    docker_tests_coverage(image_key)
                } else if (image_key != clangformat_os) {
                  docker_tests(image_key)
                }

                if (image_key == archive_what) {
                    docker_archive(image_key)
                }

                if (image_key == clangformat_os) {
                    docker_cppcheck(image_key)
                    step([$class: 'WarningsPublisher', parserConfigurations: [[parserName: 'Cppcheck Parser', pattern: "cppcheck.txt"]]])
                }
            } finally {
                sh "docker stop ${container_name(image_key)}"
                sh "docker rm -f ${container_name(image_key)}"
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

                dir("${project}") {
                    checkout scm
                }

                dir("${project}/build") {
                    sh "conan install --build=outdated .."
                    sh "cmake -DCONAN=MANUAL -DCMAKE_MACOSX_RPATH=ON .."
                    sh "make -j4"
                    sh "make -j4 unit_tests"
                    sh "make runtest"
                    sh "make runefu"
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
    // builders['macOS'] = get_macos_pipeline()

    try {
        parallel builders
    } catch (e) {
        failure_function(e, 'Job failed')
    }
}
