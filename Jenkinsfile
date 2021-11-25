@Library('ecdc-pipeline')
import ecdcpipeline.ContainerBuildNode
import ecdcpipeline.PipelineBuilder

project = "event-formation-unit"
module_src="${project}/src/modules/"
coverage_on = "centos7"
clangformat_os = "debian10"
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

container_build_nodes = [
  'centos7': ContainerBuildNode.getDefaultContainerBuildNode('centos7-gcc8'),
  'centos7-release': ContainerBuildNode.getDefaultContainerBuildNode('centos7-gcc8'),
  'debian10': ContainerBuildNode.getDefaultContainerBuildNode('debian10'),
  'ubuntu1804': ContainerBuildNode.getDefaultContainerBuildNode('ubuntu1804-gcc8')
]

def failure_function(exception_obj, failureMessage) {
    def emailmap = [ "mortenjc@jcaps.com":"morten.christensen@ess.eu", \
                     "28659574+amues@users.noreply.github.com": "afonso.mukai@ess.eu"]

    COMMITEMAIL = sh (
        script: 'git --no-pager show -s --format=\'%ae\'',
        returnStdout: true
    ).trim()

    COMMITNAME = sh (
        script: 'git --no-pager show -s --format=\'%an\'',
        returnStdout: true
    ).trim()

    EXTRATEXT="not found in mail map"
    TOMAIL=""
    if (emailmap.containsKey(COMMITEMAIL)) {
       EXTRATEXT="found in mail map"
       TOMAIL=emailmap.get(COMMITEMAIL)
    }

    def toEmails = [[$class: 'DevelopersRecipientProvider']]

    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage \
                    + '\"\n\nCheck console output at $BUILD_URL to view the results.\n\n' \
                    + 'Committer: ' + COMMITNAME + '\n' + 'Email:' + COMMITEMAIL, \
                    + '\n' + EXTRATEXT + '\n mapped to: ' + TOMAIL,
            to: 'morten.christensen@ess.eu',
            subject: '${DEFAULT_SUBJECT}'
    throw exception_obj
}

pipeline_builder = new PipelineBuilder(this, container_build_nodes, "/mnt/data:/home/jenkins/refdata")
pipeline_builder.activateEmailFailureNotifications()

builders = pipeline_builder.createBuilders { container ->

    pipeline_builder.stage("${container.key}: checkout") {
        dir(pipeline_builder.project) {
            scm_vars = checkout scm
        }
        // Copy source code to container
        container.copyTo(pipeline_builder.project, pipeline_builder.project)
    }  // stage


    if (container.key != clangformat_os) {
        pipeline_builder.stage("${container.key}: get dependencies") {
            container.sh """
                cd ${project}
                mkdir build
                cd build
                conan remote add --insert 0 ess-dmsc-local ${local_conan_server}
                conan install --build=outdated ..
            """
        }  // stage


        pipeline_builder.stage("${container.key}: configure") {
            def xtra_flags = ""
            if (container.key == coverage_on) {
                xtra_flags = "-DCOV=ON"
            } else if (container.key == archive_what) {
                xtra_flags = "-DCMAKE_BUILD_TYPE=Release -DCMAKE_SKIP_BUILD_RPATH=ON"
            }
            container.sh """
                cd ${project}/build
                . ./activate_run.sh
                cmake --version
                cmake -DREFDATA=/home/jenkins/refdata/EFU_reference -DCONAN=MANUAL -DGOOGLE_BENCHMARK=ON ${xtra_flags} ..
            """
        }  // stage

        pipeline_builder.stage("${container.key}: build") {
            container.sh """
                cd ${project}/build
                make --version
                make -j${pipeline_builder.numCpus} all unit_tests benchmark
                cd ../utils/udpredirect
                make
            """
        }  // stage
    }

    if (container.key == clangformat_os) {
        pipeline_builder.stage("${container.key}: cppcheck") {
        try {
                def test_output = "cppcheck.txt"
                // Ignore file that crashes cppcheck
                container.sh """
                                cd ${project}
                                cppcheck --enable=all --inconclusive --template="{file},{line},{severity},{id},{message}" ./ -isrc/modules/adc_readout/test/SampleProcessingTest.cpp 2> ${test_output}
                            """
                container.copyFrom("${project}", '.')
                sh "mv -f ./${project}/* ./"
            } catch (e) {
                failure_function(e, "Cppcheck step for (${container.key}) failed")
            }
        }  // stage
        step([$class: 'WarningsPublisher', parserConfigurations: [[parserName: 'Cppcheck Parser', pattern: "cppcheck.txt"]]])
    }

    if (container.key == coverage_on) {
        pipeline_builder.stage("${container.key}: test coverage") {
            abs_dir = pwd()

            try {
                container.sh """
                        cd ${project}/build
                        . ./activate_run.sh
                        make -j${pipeline_builder.numCpus} runefu
                        make coverage
                        echo skipping make -j${pipeline_builder.numCpus} valgrind
                    """
                container.copyFrom("${project}", '.')
            } catch(e) {
                container.copyFrom("${project}/build/test_results", '.')
                junit 'test_results/*.xml'
                failure_function(e, 'Run tests (${container.key}) failed')
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
        }  // stage
    } else if (container.key != clangformat_os) {
        pipeline_builder.stage("${container.key}: tests") {
            container.sh """
                cd ${project}/build
                . ./activate_run.sh
                make runtest
                make runefu
            """
        }  // stage
    }

    if (container.key == archive_what) {
        pipeline_builder.stage("${container.key}: archive") {
            container.sh """
                                mkdir -p archive/event-formation-unit
                                cp -r ${project}/build/bin archive/event-formation-unit
                                cp -r ${project}/build/modules archive/event-formation-unit
                                cp -r ${project}/build/lib archive/event-formation-unit
                                cp -r ${project}/build/licenses archive/event-formation-unit
                                mkdir archive/event-formation-unit/util
                                cp -r ${project}/utils/efushell archive/event-formation-unit/util
                                mkdir archive/event-formation-unit/configs
                                cp -r ${module_src}/multiblade/configs/* archive/event-formation-unit/configs/
                                cp -r ${module_src}/multigrid/configs/* archive/event-formation-unit/configs/
                                cp -r ${module_src}/gdgem/configs/* archive/event-formation-unit/configs/
                                cp ${project}/utils/udpredirect/udpredirect archive/event-formation-unit/util
                                mkdir archive/event-formation-unit/data

                                # Create file with build information
                                touch archive/event-formation-unit/BUILD_INFO
                                echo 'Repository: ${project}/${env.BRANCH_NAME}' >> archive/event-formation-unit/BUILD_INFO
                                echo 'Commit: ${scm_vars.GIT_COMMIT}' >> archive/event-formation-unit/BUILD_INFO
                                echo 'Jenkins build: ${BUILD_NUMBER}' >> archive/event-formation-unit/BUILD_INFO

                                cd archive
                                tar czvf event-formation-unit-centos7.tar.gz event-formation-unit
                            """
            container.copyFrom("/home/jenkins/archive/event-formation-unit-centos7.tar.gz", '.')
            container.copyFrom("/home/jenkins/archive/event-formation-unit/BUILD_INFO", '.')
            archiveArtifacts "event-formation-unit-centos7.tar.gz,BUILD_INFO"
        }
    }
}

def get_macos_pipeline()
{
    return {
        stage("macOS") {
           timestamps {
               node ("macos") {
                    // Delete workspace when build is done
                    cleanWs()

                    // temporary until all our repos have moved to using official flatbuffers and CLI11 conan packages
                    sh "conan remove -f FlatBuffers/*"
                    sh "conan remove -f cli11/*"

                    abs_dir = pwd()

                    dir("${project}") {
                        checkout scm
                    }

                    dir("${project}/build") {
                        sh "conan install --build=outdated .."
                        sh "cmake -DREFDATA=/Users/jenkins/data/EFU_reference -DCONAN=MANUAL -DCMAKE_MACOSX_RPATH=ON .."
                        sh "make -j${pipeline_builder.numCpus}"
                        sh "make -j${pipeline_builder.numCpus} unit_tests"
                        sh "make runtest"
                        sh "make runefu"
                    }
                }
            }
        }
    }
}

def get_system_tests_pipeline() {
    return {
        timestamps {
            node('system-test') {
                cleanWs()
                dir("${project}") {
                    try {
                        stage("System tests: Checkout") {
                            checkout scm
                        }  // stage
                        stage("System tests: Install requirements") {
                            sh """python3.6 -m pip install --user --upgrade pip
                            python3.6 -m pip install --user -r system-tests/requirements.txt
                            """
                        }  // stage
                        stage("System tests: Run") {
                            sh """docker stop \$(docker ps -a -q) && docker rm \$(docker ps -a -q) || true
                                                    """
                            timeout(time: 30, activity: true) {
                                sh """cd system-tests/
                                python3.6 -m pytest -s --junitxml=./SystemTestsOutput.xml ./ --pcap-file-path /mnt/data/EFU_reference/multiblade/2018_11_22/wireshark --json-file-path /mnt/data/EFU_reference/multiblade/2018_11_22/wireshark
                                """
                            }
                        }  // stage
                    } finally {
                        stage("System tests: Cleanup") {
                            sh """docker stop \$(docker ps -a -q) && docker rm \$(docker ps -a -q) || true
                            """
                        }  // stage
                        stage("System tests: Archive") {
                            junit "system-tests/SystemTestsOutput.xml"
                        }
                    }
                } // dir
            } // node
        } // timestamps
    }  // return
}  // def

// Script actions start here
timestamps {
    node('docker') {
        dir("${project}_code") {

            stage('Checkout') {
                try {
                    scm_vars = checkout scm
                    error 'MJC testing again'
                } catch (e) {
                    failure_function(e, 'Checkout failed')
                }
            }

            // skip build process if message contains '[ci skip]'
            pipeline_builder.abortBuildOnMagicCommitMessage()

            stage("Static analysis") {
                try {
                    sh "find . -name '*TestData.h' > exclude_cloc"
                    sh "find . -name 'span.hpp' >> exclude_cloc"
                    sh "find . -name 'gcovr' >> exclude_cloc"
                    sh "cloc --exclude-list-file=exclude_cloc --by-file --xml --out=cloc.xml ."
                    sh "xsltproc jenkins/cloc2sloccount.xsl cloc.xml > sloccount.sc"
                    sloccountPublish encoding: '', pattern: ''
                } catch (e) {
                    failure_function(e, 'Static analysis failed')
                }
            }
        }

        // Add macOS pipeline to builders
        builders['macOS'] = get_macos_pipeline()

        // Only add system test pipeline if this is a Pull Request
        if ( env.CHANGE_ID ) {
            builders['system tests'] = get_system_tests_pipeline()
        }

        try {
            timeout(time: 2, unit: 'HOURS') {
                // run all builders in parallel
                parallel builders
            }
        } catch (e) {
            failure_function(e, 'Job failed')
            throw e
        } finally {
            // Delete workspace when build is done
            cleanWs()
        }
    }
}
