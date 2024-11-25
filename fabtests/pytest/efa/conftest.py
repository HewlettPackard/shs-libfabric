import pytest

# The memory types for bi-directional tests.
memory_type_list_bi_dir = [
    pytest.param("host_to_host"),
    pytest.param("host_to_cuda", marks=pytest.mark.cuda_memory),
    pytest.param("cuda_to_cuda", marks=pytest.mark.cuda_memory),
    pytest.param("host_to_neuron", marks=pytest.mark.neuron_memory),
    pytest.param("neuron_to_neuron", marks=pytest.mark.neuron_memory),
]

# Add more memory types that are useful for uni-directional tests.
memory_type_list_all = memory_type_list_bi_dir + [
    pytest.param("cuda_to_host", marks=pytest.mark.cuda_memory),
    pytest.param("neuron_to_host", marks=pytest.mark.neuron_memory),
]

@pytest.fixture(scope="module", params=memory_type_list_all)
def memory_type(request):
    return request.param

@pytest.fixture(scope="module", params=memory_type_list_bi_dir)
def memory_type_bi_dir(request):
    return request.param

@pytest.fixture(scope="module", params=["read", "writedata", "write"])
def rma_operation_type(request):
    return request.param

@pytest.fixture(scope="module")
def rma_bw_memory_type(memory_type, rma_operation_type):
    is_test_bi_dir = False if rma_operation_type == "writedata" else True
    if is_test_bi_dir and (memory_type not in [_.values[0] for _ in memory_type_list_bi_dir]):
        pytest.skip("Duplicated memory type for bi-directional test")
    return memory_type


@pytest.fixture(scope="module", params=["r:0,4,64",
                                        "r:4048,4,4148",
                                        "r:8000,4,9000",
                                        "r:17000,4,18000",
                                        "r:0,1024,1048576"])
def message_size(request):
    return request.param


@pytest.fixture(scope="module", params=["r:0,4,64",
                                        "r:4048,4,4148",
                                        "r:8000,4,9000",])
def inject_message_size(request):
    return request.param


@pytest.fixture(scope="module", params=["r:0,4,32",
                                        "r:0,1024,8192",])
def zcpy_recv_message_size(request):
    return request.param

@pytest.fixture(scope="module")
def zcpy_recv_max_msg_size(request):
    return 8192

@pytest.hookimpl(hookwrapper=True)
def pytest_collection_modifyitems(session, config, items):
    # Called after collection has been performed, may filter or re-order the items in-place
    # We use this hook to always run the MR exhaustion test at the end
    mr_exhaustion_tests, other_tests = [], []
    for item in items:
        if "mr_exhaustion" in item.name:
            mr_exhaustion_tests.append(item)
        else:
            other_tests.append(item)

    yield other_tests + mr_exhaustion_tests
